#pragma once

#include "Hex.h"
#include "StoryEngine.h"

#include "Camera.h"
#include "Shader.h"
#include "Screen.h"

#include <chrono>

class Board
{
public:
	Board();
	~Board();

	bool Initialize(ID3D11Device*, int hexRadius, int cells);

	// Rendering...
	void Render(ID3D11DeviceContext*,
		Shader* shader,
		DirectX::SimpleMath::Vector3 boardPosition,
		float boardScale,
		float tileScale,
		Camera* camera, 
		float time,
		Light* light);

	void RenderUI(ID3D11DeviceContext*,
		Shader* shader,
		DirectX::SimpleMath::Vector3 boardPosition,
		float boardScale,
		Camera* camera,
		float time,
		ID3D11ShaderResourceView* texture);

	void SetInterpolation(int north, int east);
	void Interpolate(float t);

	// Changing scene...
	void Choose(int choice);
	bool Paused();

private:
	void SetInterpolationPerimeter();
	void ApplyInterpolationPermutation();

public: // FIXME: Left off while still accessed in Game.cpp...
	int m_hexRadius, m_hexDiameter, m_hexCount; // total tiles: 1+3*m_hexRadius*(m_hexRadius+1)
	int* m_hexCoordinates;
	int* m_hexPermutation;

	// Modelling info...
	Field m_horizontalField;

	float* m_hexIsolevels;
	Hex* m_hexModels; // new MarchingTerrain[1+3*m_hexRadius*(m_hexRadius+1)]

	static const DirectX::SimpleMath::Vector3 m_origin, m_p, m_q;

	// Linear interpolation...
	int m_north, m_east;

	bool m_interpolating;
	float m_t;
	DirectX::SimpleMath::Vector3 m_direction;

	// Storylets...
	StoryEngine::Scene m_scene;
	int m_sceneInterval;

	Screen m_location;

private:
	StoryEngine m_storyEngine;
};

