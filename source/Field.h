#pragma once
#include "ClassicNoise.h"
#include "SimplexNoise.h"

struct FieldVertexType
{
	DirectX::SimpleMath::Vector3 position;
	float scalar;
};

class Field
{
public:
	Field();
	~Field();

	// Initialisation...
	bool Initialise(int cells);
	bool Initialise(Field* field);

	// Planar surface generation...
	void InitialiseHorizontalField(int octaves = 8, float amplitude = 0.05f);
	void IntegrateHorizontalThorn(DirectX::SimpleMath::Vector3 prick, DirectX::SimpleMath::Vector3 root, float radius, float isolevel);
	void IntegrateShrapnel(DirectX::SimpleMath::Vector3 origin, DirectX::SimpleMath::Vector3 axis, float angle, DirectX::SimpleMath::Vector3 dimensions, float isolevel);
	void IntegrateOrb(DirectX::SimpleMath::Vector3 centre, float radius, float isolevel);

	// Spherical surface generation...
	void InitialiseSphericalField(int octaves = 8, float amplitude = 0.2f);

	// Toroidal surface generation...
	void InitialiseToroidalField(float R, int octaves = 8, float amplitude = 0.2f);

	// Cubic surface generation...
	void InitialiseCubicField();

	// Partition generation...
	void InitialisePartition(int configuration);

	// Generate *with hex*
	void DeriveHexPrism(float isolevel, bool lowerBound = true, bool upperBound = false);
	void DeriveCylindricalPrism(float isolevel, bool lowerBound = true, bool upperBound = false);

	int m_cells;
	FieldVertexType* m_field;
};