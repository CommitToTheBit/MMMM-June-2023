#include "pch.h"
#include "Screen.h"

Screen::Screen()
{

}


Screen::~Screen()
{

}

bool Screen::Initialize(ID3D11Device* device)
{
	InitializeBuffers(device);

	return true;
}

bool Screen::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	DirectX::SimpleMath::Vector3 normal, tangent, binormal;

	m_vertexCount = 4;
	m_indexCount = 6;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}


	for (int i = 0; i < 4; i++)
	{
		vertices[i].position = DirectX::SimpleMath::Vector3(pow(2.0f,0.5f)*cos((i+0.5f)*DirectX::XM_PIDIV2), pow(2.0f, 0.5f)*sin((i+0.5f)*DirectX::XM_PIDIV2), 0.0f);
		vertices[i].texture = DirectX::SimpleMath::Vector2(0.5f+0.5f*pow(2.0f, 0.5f)*cos((i+0.5f)*DirectX::XM_PIDIV2), 0.5f-0.5f*pow(2.0f, 0.5f)*sin((i+0.5f)*DirectX::XM_PIDIV2)); // NB: Check this is the right orientation!
		vertices[i].normal = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 1.0f);
		vertices[i].tangent = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 1.0f);
		vertices[i].binormal = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// Initialize the index to the vertex buffer.
	int index = 0;

	for (int i = 0; i < m_indexCount/3; i++)
	{
		indices[3*i]	= 0;
		indices[3*i+1]	= i+1;
		indices[3*i+2]	= i+2;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;
	return true;
}

void Screen::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	return;
}

void Screen::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

void Screen::Shutdown()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

void Screen::ShutdownBuffers()
{
	return;
}