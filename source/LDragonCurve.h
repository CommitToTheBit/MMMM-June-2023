#pragma once

#include "LSystem.h"

class LDragonCurve : public LSystem 
{
public:
	LDragonCurve();
	~LDragonCurve();

	bool Initialize(ID3D11Device*, float width, int iterations, float seed = 0.0f);
};

