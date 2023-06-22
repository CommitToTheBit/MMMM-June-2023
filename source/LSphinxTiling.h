#pragma once

#include "LSystem.h"

class LSphinxTiling : public LSystem
{
public:
	LSphinxTiling();
	~LSphinxTiling();

	bool Initialize(ID3D11Device*, float width, int iterations, float seed = 0.0f);
};
