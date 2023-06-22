#include "pch.h"
#include "Hex.h"

Hex::Hex()
{

}

Hex::~Hex()
{

}

void Hex::InitializeSalt(ID3D11Device* device, Field* heightField, float isolevel)
{
	m_suit = "salt";

	Field* hexField = new Field();
	hexField->Initialise(heightField);

	hexField->DeriveHexPrism(isolevel);

	__super::Initialize(device, hexField->m_cells, hexField->m_field, isolevel);

	delete hexField;
}

void Hex::InitializeThorn(ID3D11Device* device, Field* heightField, float isolevel)
{
	m_suit = "thorn";

	Field* hexField = new Field();
	hexField->Initialise(heightField);

	hexField->DeriveHexPrism(isolevel);

	int thorns = 2+std::rand()%3;
	for (int i = 0; i < thorns; i++)
	{
		float angle = XM_PIDIV2/(10.0f+2.0f*((float)std::rand()/RAND_MAX));

		float rBaseRange = 0.5f*cos(XM_PI/6.0f);
		float rBase = rBaseRange*sqrt(pow(((float)std::rand()/RAND_MAX), 2.0f)+pow(((float)std::rand()/RAND_MAX), 2.0f));
		float thetaBase = XM_2PI*std::rand()/RAND_MAX;

		DirectX::SimpleMath::Vector3 base = DirectX::SimpleMath::Vector3(0.5f+0.5f*rBase*cos(thetaBase), 0.0f, 0.5f+0.5f*rBase*sin(thetaBase));

		float rOriginRange = cos(XM_PI/6.0f);
		float rOrigin = rOriginRange*(0.5f+0.5f*sqrt(pow(((float)std::rand()/RAND_MAX), 2.0f)+pow(((float)std::rand()/RAND_MAX), 2.0f)));
		float thetaOrigin = thetaBase + 2.0f*((XM_PI/3.0f)*std::rand()/RAND_MAX);

		DirectX::SimpleMath::Vector3 origin = DirectX::SimpleMath::Vector3(0.5f+0.5f*rOrigin*cos(thetaOrigin), 0.2f+0.1f*((float)std::rand()/RAND_MAX)+isolevel, 0.5f+0.5f*rBase*sin(thetaOrigin));

		hexField->IntegrateHorizontalThorn(origin, base, angle, isolevel);
	}

	__super::Initialize(device, hexField->m_cells, hexField->m_field, isolevel);

	delete hexField;
}

void Hex::InitializeMonolith(ID3D11Device* device, Field* heightField, float isolevel)
{
	m_suit = "monolith";

	Field* hexField = new Field();
	hexField->Initialise(heightField);

	hexField->DeriveHexPrism(isolevel);

	float baseHeight = 0.66f+0.25f*((float)std::rand()/RAND_MAX);
	DirectX::SimpleMath::Vector3 baseOrigin = DirectX::SimpleMath::Vector3(0.5f, 0.5f*baseHeight, 0.5f);
	DirectX::SimpleMath::Vector3 baseAxis = DirectX::SimpleMath::Vector3(2.0f*((float)std::rand()/RAND_MAX)-1.0f, 0.01f+((float)std::rand()/RAND_MAX), 2.0f*((float)std::rand()/RAND_MAX)-1.0f);
	float baseAngle = (-30.0f+60.0f*((float)std::rand()/RAND_MAX))*(DirectX::XM_PI/180.0f);
	DirectX::SimpleMath::Vector3 baseDimensions = DirectX::SimpleMath::Vector3(0.2f+0.2f*((float)std::rand()/RAND_MAX), baseHeight, 0.2f+0.2f*((float)std::rand()/RAND_MAX));

	int variant = std::rand()%3;
	if (variant < 2)
	{
		hexField->IntegrateShrapnel(baseOrigin, baseAxis, baseAngle, baseDimensions, isolevel);
	}
	else
	{
		hexField->IntegrateShrapnel(DirectX::SimpleMath::Vector3(baseOrigin.x, 0.8f*baseOrigin.y, baseOrigin.z), baseAxis, baseAngle, DirectX::SimpleMath::Vector3(baseDimensions.x, 0.8f*baseDimensions.y, baseDimensions.z), isolevel);
		hexField->IntegrateShrapnel(DirectX::SimpleMath::Vector3(baseOrigin.x, baseOrigin.y, baseOrigin.z), baseAxis, baseAngle, DirectX::SimpleMath::Vector3(0.6f*baseDimensions.x, baseDimensions.y, 0.6f*baseDimensions.z), isolevel);
	}

	__super::Initialize(device, hexField->m_cells, hexField->m_field, isolevel);

	delete hexField;
}