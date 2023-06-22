#pragma once

#include "Field.h"

using namespace DirectX;

class MarchingCubes
{
protected:
	// General vertex information, used by shaders...
	struct VertexType
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texture;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector3 tangent;
		DirectX::SimpleMath::Vector3 binormal;
	};

public:
	MarchingCubes();
	~MarchingCubes();

	virtual bool Initialize(ID3D11Device*, int cells, FieldVertexType* field, float isolevel);
	void Render(ID3D11DeviceContext*);
	void Shutdown();

	bool Update();

private:
	bool InitializeBuffers(ID3D11Device*, int cells, FieldVertexType* field, float isolevel);
	void RenderBuffers(ID3D11DeviceContext*);
	void ShutdownBuffers();

	void CalculateIsosurfaceStatics(int* isosurfaceVertices, int* isosurfaceIndices, DirectX::SimpleMath::Vector3* isosurfacePositions, FieldVertexType* field, int cells, float isolevel); // Construction: 
	void ConstructIsosurface(VertexType* vertices, unsigned long* indices, int* isosurfaceVertices, int* isosurfaceIndices, DirectX::SimpleMath::Vector3* isosurfacePositions, int cells);

	DirectX::SimpleMath::Vector3 InterpolateIsosurface(FieldVertexType a, FieldVertexType b, float isolevel);

	// Post-processing direction vectors
	void CalculateNormalTangentBinormal(VertexType vertex1, VertexType vertex2, VertexType vertex3, DirectX::SimpleMath::Vector3& normal, DirectX::SimpleMath::Vector3& tangent, DirectX::SimpleMath::Vector3& binormal, float& weight);


protected:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;

	// Lookup tables used in the construction of the isosurface.
	static const int m_edgeTable[256];
	static const int m_triTable[256][16];
};