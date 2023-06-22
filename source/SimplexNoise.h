/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
/* This enclosed section has been adapted from: Stefan Gustavson (2005) Demystifying Simplex Noise. Available at https://weber.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf (Accessed: 10 February 2023) */

#pragma once

using namespace DirectX;

class SimplexNoise
{
public:
	SimplexNoise();
	~SimplexNoise();

	float Noise(float x, float y);
	float Noise(float x, float y, float z);

	float FBMNoise(float x, float y, int octaves = 1, float amplitude = 1.0f, float frequency = 1.0f, float damping = 0.5f);
	float FBMNoise(float x, float y, float z, int octaves = 1, float amplitude = 1.0f, float frequency = 1.0f, float damping = 0.5f);

private:
	// Mathematical functions
	int FastFloor(float x);

	float Dot(int g[2], float x, float y);
	float Dot(int g[3], float x, float y, float z);
	float Dot(int g[4], float x, float y, float z, float w);

private:
	static const int m_perm[512];
	static const int m_simplex[64][4];

	// NB: m_gradN has N*2^N entries...
	static const int m_grad2[4][2];
	static const int m_grad3[12][3];
	static const int m_grad4[32][4];
};

/* -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
