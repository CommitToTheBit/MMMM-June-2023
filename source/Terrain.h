#pragma once

using namespace DirectX;

class Terrain
{
public:
	const static int M_TERRAIN_LAYERS = 2;
private:
	// General vertex information, used by shaders...
	struct VertexType
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texture;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector3 tangent;
		DirectX::SimpleMath::Vector3 binormal;
	};

	// Simplified vertex information, used in height map calculations...
	struct HeightMapVertexType
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texture;
		DirectX::SimpleMath::Vector3 normal;
	};

	struct HeightMapType
	{
		HeightMapVertexType vertices[M_TERRAIN_LAYERS]; // NB: Using vertex type struct to condense Lab #1 code...
	};


public:
	Terrain();
	~Terrain();

	bool Initialize(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Render(ID3D11DeviceContext*);
	void Shutdown();

	// 
	bool GenerateHeightMap(ID3D11Device*, int terrainLayer);
	float RandomHeight(DirectX::SimpleMath::Vector3 position, float minHeight, float maxHeight);

	bool Update();
	float* GetWavelength();
	float* GetAmplitude();

private:
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);
	void ShutdownBuffers();

	// Post-processing direction vectors
	bool CalculateHeightMapNormals(int terrainLayer);
	void CalculateNormalTangentBinormal(VertexType vertex1, VertexType vertex2, VertexType vertex3, DirectX::SimpleMath::Vector3& normal, DirectX::SimpleMath::Vector3& tangent, DirectX::SimpleMath::Vector3& binormal);
	

private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	ID3D11Buffer * m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	float m_frequency, m_amplitude, m_wavelength;
	HeightMapType* m_heightMap;

	//arrays for our generated objects Made by directX
	std::vector<VertexPositionNormalTexture> preFabVertices;
	std::vector<uint16_t> preFabIndices;
};

