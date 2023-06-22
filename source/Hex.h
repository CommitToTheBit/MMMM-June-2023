#pragma once

#include "MarchingCubes.h"

#include <string>

class Hex : public MarchingCubes
{
public:
	Hex();
	~Hex();

	void InitializeSalt(ID3D11Device*, Field* heightField, float isolevel);
	void InitializeThorn(ID3D11Device*, Field* heightField, float isolevel);
	void InitializeMonolith(ID3D11Device*, Field* heightField, float isolevel);

public:
	std::string m_suit;

private:
	float m_seed;
};

