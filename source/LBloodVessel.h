#pragma once

#include "LSystem.h"

class LBloodVessel : public LSystem
{
public:
	LBloodVessel();
	~LBloodVessel();

	bool Initialize(ID3D11Device*, float width, int iterations, float seed = 0.0f);
};
