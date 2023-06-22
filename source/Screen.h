#pragma once

class Screen
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
	Screen();
	~Screen();

	bool Initialize(ID3D11Device*);
	void Render(ID3D11DeviceContext*);
	void Shutdown();

private:
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);
	void ShutdownBuffers();

protected:
	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;
};