#include "pch.h"
#include "Field.h"

Field::Field()
{

}


Field::~Field()
{
	delete[] m_field;
}

bool Field::Initialise(int cells)
{
	m_cells = cells;

	m_field = new FieldVertexType[(m_cells + 1) * (m_cells + 1) * (m_cells + 1)];
	for (int f = 0; f < (m_cells + 1) * (m_cells + 1) * (m_cells + 1); f++)
		m_field[f].position = DirectX::SimpleMath::Vector3(f%(m_cells+1), (f/(m_cells+1))%(m_cells+1), f/((m_cells+1)*(m_cells+1)))/m_cells;

	return true;
}

bool Field::Initialise(Field* field)
{
	m_cells = field->m_cells;

	m_field = new FieldVertexType[(m_cells + 1) * (m_cells + 1) * (m_cells + 1)];
	std::copy(field->m_field, field->m_field + (m_cells + 1) * (m_cells + 1) * (m_cells + 1), m_field);

	return true;
}

void Field::InitialiseHorizontalField(int octaves, float amplitude)
{
	SimplexNoise simplex = SimplexNoise();

	for (int f = 0; f < (m_cells+1)*(m_cells+1)*(m_cells+1); f++)
	{
		m_field[f].scalar = m_field[f].position.y;
		m_field[f].scalar += simplex.FBMNoise(m_field[f].position.x, /*10.0f**/m_field[f].position.y, m_field[f].position.z, octaves, amplitude); // NB: 10.0f factor creates some really nice, broad and barren landscapes!
		m_field[f].scalar = std::max(m_field[f].scalar, 0.0f);
	}
}

void Field::InitialiseSphericalField(int octaves, float amplitude)
{
	//m_field = new FieldVertexType[(m_cells + 1) * (m_cells + 1) * (m_cells + 1)];

	SimplexNoise simplex = SimplexNoise();

	DirectX::SimpleMath::Vector3 origin = DirectX::SimpleMath::Vector3(0.5f, 0.5f, 0.5f);

	for (int f = 0; f < (m_cells+1)*(m_cells+1)*(m_cells+1); f++)
	{
		m_field[f].scalar = 2.0f*(m_field[f].position-origin).Length();
		m_field[f].scalar += simplex.FBMNoise(m_field[f].position.x, m_field[f].position.y, m_field[f].position.z, octaves, amplitude);
		m_field[f].scalar = std::max(m_field[f].scalar, 0.0f);
	}
}

void Field::InitialiseToroidalField(float R, int octaves, float amplitude)
{
	//m_field = new FieldVertexType[(m_cells + 1) * (m_cells + 1) * (m_cells + 1)];

	SimplexNoise simplex = SimplexNoise();

	DirectX::SimpleMath::Vector3 origin = DirectX::SimpleMath::Vector3(0.5f, 0.5f, 0.5f);

	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 ringPosition;
	float r, theta, phi;

	for (int f = 0; f < (m_cells+1)*(m_cells+1)*(m_cells+1); f++)
	{
		position = 2.0f*(m_field[f].position-origin);
		r = position.Length();
		theta = atan2(position.y, position.x); // NB: Not 'true' theta...
		ringPosition = DirectX::SimpleMath::Vector3(R*cos(theta), R*sin(theta), 0.0f);

		m_field[f].scalar = (position-ringPosition).Length();
		m_field[f].scalar += simplex.FBMNoise(m_field[f].position.x, m_field[f].position.y, m_field[f].position.z, octaves, std::min(R, 1.0f-R)*amplitude);
		m_field[f].scalar /= std::min(R, 1.0f-R);
		m_field[f].scalar = std::max(m_field[f].scalar, 0.0f);
	}
}

void Field::InitialiseCubicField()
{
	DirectX::SimpleMath::Vector3 origin = DirectX::SimpleMath::Vector3(0.5f, 0.5f, 0.5f);

	DirectX::SimpleMath::Vector3 position;
	for (int f = 0; f < (m_cells+1)*(m_cells+1)*(m_cells+1); f++)
	{
		position = 2.0f*(m_field[f].position-origin);

		m_field[f].scalar = std::max(abs(position.x), std::max(abs(position.y), abs(position.z)));
	}
}

void Field::IntegrateHorizontalThorn(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 base, float angle, float isolevel)
{
	float theta, thorn;
	for (int f = 0; f < (m_cells+1)*(m_cells+1)*(m_cells+1); f++)
	{
		theta = acos((m_field[f].position-origin).Dot(base-origin)/((m_field[f].position-origin).Length()*(base-origin).Length()));
		thorn = theta/angle;

		// FIXME: Procedural coarseness is not convincing...
		//thorn -= simplex.FBMNoise(m_field[f].position.x, m_field[f].position.y, m_field[f].position.z, 8, 0.5f);

		m_field[f].scalar = std::min(m_field[f].scalar, isolevel*thorn); // NB: Use of min, since this points 'out' from isosurface...
	}
}

void Field::IntegrateShrapnel(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 axis, float angle, DirectX::SimpleMath::Vector3 dimensions, float isolevel)
{
	DirectX::SimpleMath::Vector3 position;
	float shrapnel;
	for (int f = 0; f < (m_cells+1)*(m_cells+1)*(m_cells+1); f++)
	{
		position = 2.0f*(m_field[f].position-origin);
		DirectX::SimpleMath::Vector3::Transform(position, DirectX::SimpleMath::Matrix::CreateFromAxisAngle(axis, angle), position); // FIXME: Hacky use of rotations?
		shrapnel = std::max((1.0f+m_field[f].position.y)*abs(position.x/dimensions.x), std::max(abs(position.y/dimensions.y), (1.0f+m_field[f].position.y)*abs(position.z/dimensions.z)));

		m_field[f].scalar = std::min(m_field[f].scalar, isolevel*shrapnel);
	}
}


void Field::InitialisePartition(int configuration)
{
	Initialise(1);
	configuration %= 256;

	for (int i = 0; i < 8; i++)
		//m_field[i].scalar = ((configuration%((int)pow(2, i)))/(pow(2, i-1)) == 0) ? -1.0f : 1.0f;
		m_field[i].scalar = (configuration & (int)pow(2, i)) ? -1.0f : 1.0f;
}

void Field::IntegrateOrb(DirectX::SimpleMath::Vector3 centre, float radius, float isolevel)
{
	SimplexNoise simplex = SimplexNoise();

	float orb;

	for (int f = 0; f < m_cells * m_cells * m_cells; f++)
	{
		orb = (m_field[f].position-centre).Length()/radius;

		// FIXME: Procedural coarseness is not convincing...
		//orb += simplex.FBMNoise(m_field[f].position.x, m_field[f].position.y, m_field[f].position.z, 8, 0.2f*radius);

		m_field[f].scalar = std::min(m_field[f].scalar, isolevel*orb);
	}
}

void Field::DeriveHexPrism(float isolevel, bool lowerBound, bool upperBound)
{
	DirectX::SimpleMath::Vector2 position;
	float r, theta, modTheta, z;

	int q, quadrant;
	DirectX::SimpleMath::Vector2 quadrantDirection;

	for (int f = 0; f < (m_cells+1)*(m_cells+1)*(m_cells+1); f++)
	{
		position = 2.0f*DirectX::SimpleMath::Vector2(m_field[f].position.x-0.5f, m_field[f].position.z-0.5f);

		theta = atan2(position.y, position.x);
		if (theta < 0.0f)
			theta += XM_2PI;

		modTheta = theta;
		while (modTheta >= XM_PI/3)
			modTheta -= XM_PI/3;

		r = (cos(modTheta-XM_PI/6)/cos(XM_PI/6))*position.Length();
		m_field[f].scalar = std::max(m_field[f].scalar, isolevel*r);

		if (lowerBound)
		{
			z = 1.0f-m_field[f].position.y;
			m_field[f].scalar = std::max(m_field[f].scalar, isolevel*z);
		}

		if (upperBound)
		{
			z = m_field[f].position.y;
			m_field[f].scalar = std::max(m_field[f].scalar, isolevel*z);
		}
	}
}

void Field::DeriveCylindricalPrism(float isolevel, bool lowerBound, bool upperBound)
{
	DirectX::SimpleMath::Vector2 position;
	float r, theta, z;

	int q, quadrant;
	DirectX::SimpleMath::Vector2 quadrantDirection;

	for (int f = 0; f < (m_cells+1)*(m_cells+1)*(m_cells+1); f++)
	{
		position = 2.0f*DirectX::SimpleMath::Vector2(m_field[f].position.x-0.5f, m_field[f].position.z-0.5f);
		r = position.Length();
		m_field[f].scalar = std::max(m_field[f].scalar, isolevel*r);

		if (lowerBound)
		{
			z = 1.0f-m_field[f].position.y;
			m_field[f].scalar = std::max(m_field[f].scalar, isolevel*z);
		}

		if (upperBound)
		{
			z = m_field[f].position.y;
			m_field[f].scalar = std::max(m_field[f].scalar, isolevel*z);
		}
	}
}