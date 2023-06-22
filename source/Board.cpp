#include "pch.h"
#include "Board.h"

const DirectX::SimpleMath::Vector3 Board::m_origin = DirectX::SimpleMath::Vector3(-0.5f, 0.0f, -0.5f);
const DirectX::SimpleMath::Vector3 Board::m_p = DirectX::SimpleMath::Vector3(0.5f*(1.0f+cos(XM_PI/3.0f)), 0.0f, -0.5f*cos(XM_PI/6.0f)); // Points NE...
const DirectX::SimpleMath::Vector3 Board::m_q = DirectX::SimpleMath::Vector3(-0.5f*(1.0f+cos(XM_PI/3.0f)), 0.0f, -0.5f*cos(XM_PI/6.0f)); // Points NW...

Board::Board()
{
	std::srand(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

	m_interpolating = false;

}


Board::~Board()
{

}

bool Board::Initialize(ID3D11Device* device, int hexRadius, int cells)
{
	hexRadius = std::max(hexRadius, 1);

	m_hexRadius = hexRadius;
	m_hexDiameter = (2*hexRadius+1);
	m_hexCount = 1+3*hexRadius*(hexRadius+1); // NB: We add an 'outer ring' of repeated hexes

	m_hexCoordinates = new int[(m_hexDiameter+2)*(m_hexDiameter+2)];
	m_hexPermutation = new int[m_hexCount];

	m_hexIsolevels = new float[m_hexCount];
	m_hexModels = new Hex[m_hexCount];

	m_horizontalField.Initialise(cells);
	m_horizontalField.InitialiseHorizontalField(6,0.1f);

	int index = 0;
	for (int j = -m_hexRadius-1; j <= m_hexRadius+1; j++)
	{
		for (int i = -m_hexRadius-1; i <= m_hexRadius+1; i++)
		{
			if (abs(i) > m_hexRadius || abs(i-j) > m_hexRadius || abs(j) > m_hexRadius)
			{
				m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1] = 0;
				continue;
			}

			m_hexIsolevels[index] = 0.15f+0.15f*std::rand()/RAND_MAX;

			int landmark = std::rand()%15;
			if (landmark < 2)
			{
				m_hexModels[index].InitializeThorn(device, &m_horizontalField, m_hexIsolevels[index]);
			}
			else if (landmark < 3)
			{
				m_hexModels[index].InitializeMonolith(device, &m_horizontalField, m_hexIsolevels[index]);
			}
			else
			{
				m_hexModels[index].InitializeSalt(device, &m_horizontalField, m_hexIsolevels[index]);
			}

			m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1] = index;
			m_hexPermutation[index] = index;// (index+m_hexes/2)%m_hexes;


			index++;
		}
	}
	//SetPerimeter();

	m_north = 0;
	m_east = 0;

	m_t = 0.0f;
	m_direction = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

	//delete hexField;

	// Storylets...
	m_storyEngine.Initialize(std::rand());

	int playerIndex = m_hexPermutation[m_hexCoordinates[2*(m_hexDiameter+2)+2]]; // NB: Player's current hex...
	m_scene = m_storyEngine.StartScene(m_hexModels[playerIndex].m_suit);
	m_sceneInterval = 0;

	m_location.Initialize(device);

	return true;
}

void Board::Render(ID3D11DeviceContext* deviceContext, Shader* shader, DirectX::SimpleMath::Vector3 boardPosition, float boardScale, float tileScale, Camera* camera, float time, Light* light) // NB: Initlaise shader as part of Board?
{
	float ifrac = -m_t*((m_east != -m_north) ? m_north : 0);
	float jfrac = -m_t*((m_east != m_north) ? m_north : 0);

	DirectX::SimpleMath::Vector3 relativePosition, boundPosition;
	float maxBound, tBound;
	for (int j = m_hexRadius+1; j >= -m_hexRadius-1; j--)
	{
		for (int i = m_hexRadius+1; i >= -m_hexRadius-1; i--)
		{
			if (abs(i-j) > m_hexRadius+1)
				continue;

			// FIXME: Refactor this, for 'cleaner' board set-up?
			float l = (camera->getPosition()-boardPosition).Length();
			DirectX::SimpleMath::Matrix ortho = DirectX::SimpleMath::Matrix::CreateOrthographic(l*1920.0f/1080.0f, l*1.0f, 0.01f, 100.0f);

			relativePosition = (i+ifrac)*m_p+(j+jfrac)*m_q;

			// FIXME: Needs tidied!
			maxBound = std::max(std::max(abs(i+ifrac), abs(i+ifrac-j-jfrac)), abs(j+jfrac));
			tBound = std::max(0.0f, std::min(m_hexRadius + 1.0f - maxBound, 1.0f));
			boundPosition = DirectX::SimpleMath::Vector3(0.0f, -0.5f*(1.0f - tBound), 0.0f);

			shader->EnableShader(deviceContext);
			shader->SetMatrixBuffer(deviceContext, &(DirectX::SimpleMath::Matrix::CreateTranslation(m_origin) * DirectX::SimpleMath::Matrix::CreateScale(tileScale) * DirectX::SimpleMath::Matrix::CreateTranslation(boardPosition+relativePosition+boundPosition) * DirectX::SimpleMath::Matrix::CreateScale(boardScale)), &camera->getCameraMatrix(), &ortho, true);
			shader->SetAlphaBuffer(deviceContext, tBound);
			shader->SetLightBuffer(deviceContext, light);
			// Textures, then...
			m_hexModels[m_hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1]]].Render(deviceContext);
		}
	}

	return;
}

void Board::RenderUI(ID3D11DeviceContext* deviceContext, Shader* shader, DirectX::SimpleMath::Vector3 boardPosition, float boardScale, Camera* camera, float time, ID3D11ShaderResourceView* texture)
{
	DirectX::SimpleMath::Vector3 relativePosition = (-m_hexRadius+1)*m_p+(-m_hexRadius+1)*m_q+DirectX::SimpleMath::Vector3(0.0f, 0.6f+0.025f*sin(time), 0.0f);

	float l = (camera->getPosition()-boardPosition).Length();
	DirectX::SimpleMath::Matrix ortho = DirectX::SimpleMath::Matrix::CreateOrthographic(l*1920.0f/1080.0f, l*1.0f, 0.01f, 100.0f);

	shader->EnableShader(deviceContext);
	shader->SetMatrixBuffer(deviceContext, &(DirectX::SimpleMath::Matrix::CreateTranslation(0.5f, 0.0f, 0.25f) * DirectX::SimpleMath::Matrix::CreateTranslation(m_origin) * DirectX::SimpleMath::Matrix::CreateScale(0.25f) * DirectX::SimpleMath::Matrix::CreateTranslation(boardPosition+relativePosition) * DirectX::SimpleMath::Matrix::CreateScale(boardScale)), &camera->getCameraMatrix(), &ortho, true);
	shader->SetShaderTexture(deviceContext, texture, 0, 0);
	m_location.Render(deviceContext);
}

void Board::SetInterpolation(int north, int east)
{
	// ERROR-HANDLING: 'Normalise' direction...
	m_north = (north != 0) ? north/abs(north) : 0;
	m_east = (east != 0) ? east/abs(east) : 0;

	if (m_north == 0)
		return;

	m_interpolating = true;
	SetInterpolationPerimeter();
}

void Board::Interpolate(float t)
{
	m_t += t;
	if (m_t < 1.0f)
		return;

	ApplyInterpolationPermutation();

	m_interpolating = false;
	m_t = 0.0f;
	m_north = 0;
	m_east = 0;

	int playerIndex = m_hexPermutation[m_hexCoordinates[2*(m_hexDiameter+2)+2]]; // NB: Player's current hex...
	if (m_hexModels[playerIndex].m_suit != "salt" || ++m_sceneInterval >= 7-m_storyEngine.GetPartySize()) // NB: A larger party size means more frequent events of interest!
	{
		m_scene = m_storyEngine.StartScene(m_hexModels[playerIndex].m_suit);
		m_sceneInterval = 0;
	}
}

void Board::SetInterpolationPerimeter()
{
	int ijColumn, ijColumnLength;
	for (int j = -m_hexRadius-1; j <= m_hexRadius+1; j++)
	{
		for (int i = -m_hexRadius-1; i <= m_hexRadius+1; i++)
		{
			if (m_east == -m_north)
			{
				ijColumn = i;
				ijColumnLength = m_hexDiameter-abs(ijColumn);

				if (abs(i) < m_hexRadius+1 && (i-j == -m_north*(m_hexRadius+1)) || j == m_north*(m_hexRadius+1)) // NB: Minus sign here due to ij/NESW coordinate systems!
					m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1] = m_hexCoordinates[(m_hexDiameter+2)*((j-m_north*ijColumnLength)+m_hexRadius+1)+i+m_hexRadius+1];
			}
			else if (m_east == 0)
			{
				ijColumn = i-j;
				ijColumnLength = m_hexDiameter-abs(ijColumn);

				if (abs(i-j) < m_hexRadius+1 && (i == m_north*(m_hexRadius+1) || j == m_north*(m_hexRadius+1)))
					m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1] = m_hexCoordinates[(m_hexDiameter+2)*((j-m_north*ijColumnLength)+m_hexRadius+1)+(i-m_north*ijColumnLength)+m_hexRadius+1];
			}
			else
			{
				ijColumn = j;
				ijColumnLength = m_hexDiameter-abs(ijColumn);

				if (abs(j) < m_hexRadius+1 && (i == m_north*(m_hexRadius+1) || i-j == m_north*(m_hexRadius+1))) // NB: Minus sign here due to ij/NESW coordinate systems!
					m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1] = m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+(i-m_north*ijColumnLength)+m_hexRadius+1];
			}
		}
	}

	return;

	int sign;
	for (int j = -m_hexRadius-1; j <= m_hexRadius+1; j++)
	{
		for (int i = -m_hexRadius-1; i <= m_hexRadius+1; i++)
		{
			if (abs(i) < m_hexRadius+1 && (abs(i-j) == m_hexRadius+1 || abs(j) == m_hexRadius+1))
			{
				ijColumn = i;
				ijColumnLength = m_hexDiameter-abs(ijColumn);

				m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1] = m_hexCoordinates[(m_hexDiameter+2)*(j-(j/abs(j))*ijColumnLength+m_hexRadius+1)+i+m_hexRadius+1];
			}
			else if (abs(j) < m_hexRadius+1 && (abs(i) == m_hexRadius+1) || abs(i-j) == m_hexRadius+1)
			{
				ijColumn = j;
				ijColumnLength = m_hexDiameter-abs(ijColumn);

				m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1] = m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i-(i/abs(i))*ijColumnLength+m_hexRadius+1];
			}
		}
	}

	// NB: Special 'verticcal' cases, not handled in the above loop...
	m_hexCoordinates[0] = m_hexCoordinates[(m_hexDiameter+2)*m_hexDiameter+m_hexDiameter];
	m_hexCoordinates[(m_hexDiameter+2)*(m_hexDiameter+2)-1] = m_hexCoordinates[(m_hexDiameter+2)+1];
}

void Board::ApplyInterpolationPermutation()
{
	if (m_north == 0)
		return;

	int* hexPermutation = new int[m_hexCount];

	int ijColumn, ijColumnLength;
	int	iPermuted, jPermuted;
	for (int j = -m_hexRadius; j <= m_hexRadius; j++)
	{
		for (int i = -m_hexRadius; i <= m_hexRadius; i++)
		{
			if (abs(i-j) > m_hexRadius)
				continue;

			if (m_east == -m_north)
				ijColumn = i;
			else if (m_east == 0)
				ijColumn = i-j;
			else
				ijColumn = j;
			ijColumnLength = m_hexDiameter-abs(ijColumn);

			if (m_east == -m_north)
			{
				if (i-j == -m_north*m_hexRadius || j == m_north*m_hexRadius)
					hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1]] = m_hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j-m_north*(ijColumnLength-1)+m_hexRadius+1)+i+m_hexRadius+1]];
				else
					hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1]] = m_hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_north+m_hexRadius+1)+i+m_hexRadius+1]];
			}
			else if (m_east == 0)
			{
				if (i == m_north*m_hexRadius || j == m_north*m_hexRadius)
					hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1]] = m_hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j-m_north*(ijColumnLength-1)+m_hexRadius+1)+i-m_north*(ijColumnLength-1)+m_hexRadius+1]];
				else
					hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1]] = m_hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_north+m_hexRadius+1)+i+m_north+m_hexRadius+1]];
			}
			else
			{
				if (i-j == m_north*m_hexRadius || i == m_north*m_hexRadius)
					hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1]] = m_hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i-m_north*(ijColumnLength-1)+m_hexRadius+1]];
				else
					hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_hexRadius+1]] = m_hexPermutation[m_hexCoordinates[(m_hexDiameter+2)*(j+m_hexRadius+1)+i+m_north+m_hexRadius+1]];
			}
		}
	}

	m_hexPermutation = hexPermutation;
}

void Board::Choose(int choice)
{
	m_scene = m_storyEngine.ContinueScene(choice);
}

bool Board::Paused()
{
	return m_scene.choices.size() > 0;
}