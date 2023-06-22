#include "pch.h"
#include "Terrain.h"


Terrain::Terrain()
{
	m_terrainGeneratedToggle = false;
}


Terrain::~Terrain()
{
}

bool Terrain::Initialize(ID3D11Device* device, int terrainWidth, int terrainHeight)
{
	int index;
	float height = 0.0;
	bool result;

	// Save the dimensions of the terrain.
	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;

	m_frequency = m_terrainWidth / 20;
	m_amplitude = 16.0f;
	m_wavelength = 0.7f;

	// Create the structure to hold the terrain data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_heightMap)
	{
		return false;
	}

	//this is how we calculate the texture coordinates first calculate the step size there will be between vertices. 
	float textureCoordinatesStep = 3.0f / m_terrainWidth;  //tile n times across the terrain. 
	// Initialise the data in the height map (flat).
	for (int j = 0; j<m_terrainHeight; j++)
	{
		for (int i = 0; i<m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].vertices[0].position = DirectX::SimpleMath::Vector3((float)i, (float)height, (float)j);
			m_heightMap[index].vertices[1].position = DirectX::SimpleMath::Vector3((float)i, (float)height, (float)j);

			//and use this step to calculate the texture coordinates for this point on the terrain.
			m_heightMap[index].vertices[0].texture = textureCoordinatesStep * DirectX::SimpleMath::Vector2((float)i, (float)j);
			m_heightMap[index].vertices[1].texture = textureCoordinatesStep * DirectX::SimpleMath::Vector2((float)i, (float)j);

		}
	}

	//even though we are generating a flat terrain, we still need to normalise it. 
	// Calculate the normals for the terrain data.
	for (int terrainLayer = 0; terrainLayer < M_TERRAIN_LAYERS; terrainLayer++)
	{
		result = CalculateHeightMapNormals(terrainLayer);
		if (!result)
		{
			return false;
		}
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}


	return true;
}

bool Terrain::InitializeBuffers(ID3D11Device * device )
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int index;
	int heightMapIndices[6];
	DirectX::SimpleMath::Vector3 normal, tangent, binormal;

	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// Initialize the index to the vertex buffer.
	index = 0;

	for (int j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (int i = 0; i < (m_terrainWidth - 1); i++)
		{
			heightMapIndices[0] = (m_terrainHeight * j) + i;          // Bottom left.
			heightMapIndices[1] = (m_terrainHeight * (j + 1)) + i;      // Upper left.
			heightMapIndices[2] = (m_terrainHeight * (j + 1)) + (i + 1);  // Upper right.
			heightMapIndices[3] = (m_terrainHeight * j) + i;          // Bottom left.
			heightMapIndices[4] = (m_terrainHeight * (j + 1)) + (i + 1);  // Upper right.
			heightMapIndices[5] = (m_terrainHeight * j) + (i + 1);      // Bottom right.

			// Front Faces: Bottom left -> Upper left -> Upper right; Bottom left -> Upper right -> Bottom right
			for (int k = 0; k < 6; k++)
			{
				vertices[index].position = m_heightMap[heightMapIndices[k]].vertices[M_TERRAIN_LAYERS - 1].position;
				vertices[index].texture = m_heightMap[heightMapIndices[k]].vertices[M_TERRAIN_LAYERS - 1].texture;
				indices[index] = index;
				index++;
			}
			index -= 6;

			for (int k = 0; k < 2; k++)
			{
				CalculateNormalTangentBinormal(vertices[index], vertices[index+1], vertices[index+2], normal, tangent, binormal);

				for (int l = 0; l < 3; l++)
				{
					vertices[index].normal = normal;
					vertices[index].tangent = tangent;
					vertices[index].binormal = binormal;
					index++;
				}
			}
		}
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

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void Terrain::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	return;
}

void Terrain::RenderBuffers(ID3D11DeviceContext * deviceContext)
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

void Terrain::Shutdown()
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

void Terrain::ShutdownBuffers()
{
	return;
}


/* HEIGHT MAP GENERATION */ 
bool Terrain::GenerateHeightMap(ID3D11Device* device, int terrainLayer)
{
	bool result;

	int index;
	float height = 0.0;

	DirectX::SimpleMath::Vector3 init_position, init_normal;

	m_frequency = (6.283/m_terrainHeight) / m_wavelength; //we want a wavelength of 1 to be a single wave over the whole terrain.  A single wave is 2 pi which is about 6.283

	//loop through the terrain and set the hieghts how we want. This is where we generate the terrain
	//in this case I will run a sin-wave through the terrain in one axis.

	for (int j = 0; j<m_terrainHeight; j++)
	{
		for (int i = 0; i<m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			if (terrainLayer == 0)
				m_heightMap[index].vertices[terrainLayer].position = DirectX::SimpleMath::Vector3((float)i, (float)height, (float)j) + sin((float)i *(m_frequency))*m_amplitude * DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
			else if (terrainLayer == 1)
				m_heightMap[index].vertices[terrainLayer].position = m_heightMap[index].vertices[terrainLayer-1].position + RandomHeight(m_heightMap[index].vertices[terrainLayer-1].position, 0.0f, 0.0f*m_amplitude*m_wavelength) * m_heightMap[index].vertices[terrainLayer-1].normal;
		}
	}

	result = CalculateHeightMapNormals(terrainLayer);
	if (!result)
	{
		return false;
	}

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
}

float Terrain::RandomHeight(DirectX::SimpleMath::Vector3 position, float minHeight, float maxHeight)
{
	float random = 34227.56*sin(256.3*position.x + 444.7*position.z);
	//float t = new RandomN(position)

	float t = (float)rand()/RAND_MAX;

	return (1.0f-t)*minHeight + t*maxHeight;
}

bool Terrain::CalculateHeightMapNormals(int terrainLayer)
{
	int index;
	int indices[3];

	DirectX::SimpleMath::Vector3* heightMapNormals;

	DirectX::SimpleMath::Vector3 heightMapBinormal;
	DirectX::SimpleMath::Vector3 heightMapTangent;
	DirectX::SimpleMath::Vector3 heightMapNormal;

	// Create a temporary array to hold the un-normalized normal vectors.
	heightMapNormals = new DirectX::SimpleMath::Vector3[m_terrainHeight * m_terrainWidth];
	if (!heightMapNormals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normal, tangent, binormal.
	for (int j = 0; j < m_terrainHeight - 1; j++)
	{
		for (int i = 0; i < m_terrainWidth - 1; i++)
		{
			indices[0] = (j * m_terrainHeight) + i;
			indices[1] = (j * m_terrainHeight) + (i + 1); // Step in u-direction
			indices[2] = ((j + 1) * m_terrainHeight) + i; // Step in v-direction

			heightMapBinormal = m_heightMap[indices[1]].vertices[terrainLayer].position - m_heightMap[indices[0]].vertices[terrainLayer].position;
			heightMapTangent = m_heightMap[indices[2]].vertices[terrainLayer].position - m_heightMap[indices[0]].vertices[terrainLayer].position;
			heightMapNormals[(j * (m_terrainHeight - 1)) + i] = heightMapTangent.Cross(heightMapBinormal);
		}
	}

	// Now go through all the vertices and take an average of each face normal
	// that the vertex touches to get the averaged normal for that vertex.
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			heightMapNormal = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f); // Start at zero...

			if (i - 1 >= 0 && j - 1 >= 0)
				heightMapNormal += heightMapNormals[((j - 1) * (m_terrainHeight - 1)) + (i - 1)]; // Add normal from bottom left face...

			if (i < m_terrainWidth - 1 && j - 1 >= 0)
				heightMapNormal += heightMapNormals[((j - 1) * (m_terrainHeight - 1)) + i]; // Add normal from bottom right face...

			if (i - 1 >= 0 && j < m_terrainHeight - 1)
				heightMapNormal += heightMapNormals[(j * (m_terrainHeight - 1)) + (i - 1)]; // Add normal from upper left face...

			if (i < m_terrainWidth - 1 && j < m_terrainHeight - 1)
				heightMapNormal += heightMapNormals[(j * (m_terrainHeight - 1)) + i]; // Add normal from upper right face...

			heightMapNormal.Normalize();

			if (heightMapNormal.Length() == 0.0f)
				heightMapNormal = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);

			m_heightMap[(j * m_terrainHeight) + i].vertices[terrainLayer].normal = heightMapNormal;
		}
	}

	// Release the temporary normals.
	delete[] heightMapNormals;
	heightMapNormals = 0;

	return true;
}

void Terrain::CalculateNormalTangentBinormal(VertexType vertex1, VertexType vertex2, VertexType vertex3, DirectX::SimpleMath::Vector3& normal, DirectX::SimpleMath::Vector3& tangent, DirectX::SimpleMath::Vector3& binormal)
{
	/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */
	/* This enclosed section has been adapted from: RasterTek (no date) Tutorial 20: Bump Mapping. Available at https://www.rastertek.com/dx11tut20.html (Accessed: 28 December 2022) */

	DirectX::SimpleMath::Vector3 vector1, vector2;
	DirectX::SimpleMath::Vector2 textureVector1, textureVector2;
	float determinant;
	float length;

	// Calculate the two vectors for this face.
	vector1 = (DirectX::SimpleMath::Vector3)vertex2.position - vertex1.position;
	vector2 = (DirectX::SimpleMath::Vector3)vertex3.position - vertex1.position;

	// Calculate the tu and tv texture space vectors.
	textureVector1.x = vertex2.texture.x - vertex1.texture.x;
	textureVector1.y = vertex2.texture.y - vertex1.texture.y;

	textureVector2.x = vertex3.texture.x - vertex1.texture.x;
	textureVector2.y = vertex3.texture.y - vertex1.texture.y;

	// Calculate the denominator of the tangent/binormal equation.
	determinant = textureVector1.x * textureVector2.y - textureVector1.y * textureVector2.x;

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent = (textureVector2.y*vector1 - textureVector1.y*vector2) / determinant;
	binormal = (textureVector2.x*vector1 - textureVector1.x*vector2) / determinant;

	// Normalise tangent and binormal
	tangent.Normalize();
	binormal.Normalize();

	if (tangent.Length() == 0)
		tangent = DirectX::SimpleMath::Vector3(1.0, 0.0, 0.0);

	if (binormal.Length() == 0)
		binormal = DirectX::SimpleMath::Vector3(0.0, 0.0, 1.0);

	// Calculate normal
	normal = tangent.Cross(binormal); // NB: Note the orientation of the vector space!

	return;

	/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ */
}

bool Terrain::Update()
{
	return true; 
}

float* Terrain::GetWavelength()
{
	return &m_wavelength;
}

float* Terrain::GetAmplitude()
{
	return &m_amplitude;
}
