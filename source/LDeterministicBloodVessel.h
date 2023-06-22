#pragma once

#include "LSystem.h"

class LDeterministicBloodVessel : public LSystem
{
public:
	LDeterministicBloodVessel();
	~LDeterministicBloodVessel();

	bool Initialize(ID3D11Device*, float width, float asymmetry, int iterations, float seed = 0.0f);
};
