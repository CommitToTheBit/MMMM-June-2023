#pragma once
#include "LSystem.h"

class LPenroseP3: public LSystem
{
public:
	LPenroseP3();
	~LPenroseP3();

	bool Initialize(ID3D11Device*, float width, int iterations, float seed = 0.0f);
};
