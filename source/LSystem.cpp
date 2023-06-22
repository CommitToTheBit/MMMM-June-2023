#include "pch.h"
#include "LSystem.h"

LSystem::LSystem()
{

}


LSystem::~LSystem()
{

}

LSystem::LModuleType::LModuleType()
{
	letter = "";

	period = 0.0f;
	aperiodicity = 0.0f;
	synchronisation = 0.0f;
	asynchronicity = 0.0f;

	staticLength = 0.0f;
	randomStaticLength = 0.0f;
	periodicLength = 0.0f;
	randomPeriodicLength = 0.0f;

	staticRotation = 0.0f;
	randomStaticRotation = 0.0f;
	periodicRotation = 0.0f;
	randomPeriodicRotation = 0.0f;

	staticWidth = 0.0f;
	randomStaticWidth = 0.0f;
	periodicWidth = 0.0f;
	randomPeriodicWidth = 0.0f;

	asymmetry = 0.0f;
	staticAsymmetry = 0.0f;
	randomStaticAsymmetry = 0.0f;
	periodicAsymmetry = 0.0f;
	randomPeriodicAsymmetry = 0.0f;
}

bool LSystem::Initialize(ID3D11Device* device, std::vector<LModuleType> axiom, int iterations, float seed, float rotation, DirectX::SimpleMath::Vector2 anchoring)
{
	// STEP 1: Initialize sentence, using grammar...
	InitializeSentence(seed, axiom, iterations);

	// STEP 2: Initialize tree, to scale...
	InitializeTree(seed, rotation, anchoring);

	// STEP 3:
	InitializeBuffers(device);

	return true;
}

void LSystem::InitializeSentence(float seed, std::vector<LModuleType> axiom, int iterations)
{
	srand(seed);

	m_sentence = axiom;
	for (int i = 0; i < axiom.size(); i++)
	{
		// DEBUG?
		// CHECKME: Why does this *need* initialised??
		m_sentence[i].asymmetry = m_sentence[i].staticAsymmetry+GetRNGRange()*m_sentence[i].randomStaticAsymmetry;
	}

	float asymmetry;

	std::vector<LModuleType> iteratedSentence;
	for (int i = 0; i < iterations; i++)
	{
		iteratedSentence = std::vector<LModuleType>();

		for each (LModuleType LModule in m_sentence)
		{
			LModuleType KModule = LModule;
			for each (std::function<LModuleType(LModuleType)> production in GetProductionRule(LModule.letter).productions)
			{
				iteratedSentence.push_back(production(LModule));

				// DEBUG?
				iteratedSentence.at(iteratedSentence.size()-1).asymmetry = iteratedSentence.at(iteratedSentence.size()-1).staticAsymmetry+GetRNGRange()*iteratedSentence.at(iteratedSentence.size()-1).randomStaticAsymmetry;
			}
		}

		m_sentence = iteratedSentence;
	}
}

void LSystem::InitializeTree(float seed, float rotation, DirectX::SimpleMath::Vector2 anchoring)
{
	m_seed = seed;
	m_rotation = rotation;
	m_scale = 1.0f;
	m_depth = 0.0f;
	m_time = 0.0f;
	m_intensity = 0.0f;

	m_seedVertices = std::vector<SeedVertexType>();

	m_seedVertices.push_back(SeedVertexType());
	m_seedVertices[0].parent = 0;
	m_seedVertices[0].transform = DirectX::SimpleMath::Matrix::CreateRotationZ(m_rotation);
	m_seedVertices[0].position = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

	m_seedVertices[0].depth = 0.0f;

	int parentIndex = 0;
	std::vector<int> parentIndices = std::vector<int>();

	DirectX::SimpleMath::Matrix localTransform = DirectX::SimpleMath::Matrix::Identity;

	// STEP 1: Create branching structure...
	int childIndex = 1;
	for each (LModuleType LModule in m_sentence)
	{
		if (LModule.letter == "[")
		{
			parentIndices.push_back(parentIndex);
		}
		else if (LModule.letter == "]" && parentIndices.size() > 0)
		{
			parentIndex = parentIndices[parentIndices.size()-1];
			parentIndices.pop_back();
		}
		else
		{
			localTransform = DirectX::SimpleMath::Matrix::CreateRotationZ(LModule.staticRotation) * localTransform;

			if (LModule.staticLength == 0.0f)
				continue;

			localTransform = DirectX::SimpleMath::Matrix::CreateTranslation(LModule.staticLength, 0.0f, 0.0f) * localTransform;

			m_seedVertices.push_back(SeedVertexType());
			m_seedVertices[childIndex].parent = parentIndex;
			m_seedVertices[childIndex].transform = localTransform * m_seedVertices[parentIndex].transform;
			DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), m_seedVertices[childIndex].transform, m_seedVertices[childIndex].position);

			m_seedVertices[parentIndex].depth += LModule.staticLength; // NB: Really interesting quirk of design - accidentally writing ParentIndex on the LHS produces a really nice 'sprouting' effect!
			// FIXME: Include a 'simplex value' here?

			parentIndex = childIndex;
			childIndex = m_seedVertices.size();
			localTransform = DirectX::SimpleMath::Matrix::Identity;
		}
	}

	// STEP 2: Calculate bounds...
	DirectX::SimpleMath::Vector2 minima = DirectX::SimpleMath::Vector2(0.0f, 0.0f);
	DirectX::SimpleMath::Vector2 maxima = DirectX::SimpleMath::Vector2(0.0f, 0.0f);
	for (int i = 0; i < m_seedVertices.size(); i++)
	{
		if (m_seedVertices[i].position.x < minima.x)
			minima.x = m_seedVertices[i].position.x;
		else if (m_seedVertices[i].position.x > maxima.x)
			maxima.x = m_seedVertices[i].position.x;

		if (m_seedVertices[i].position.y < minima.y)
			minima.y = m_seedVertices[i].position.y;
		else if (m_seedVertices[i].position.y > maxima.y)
			maxima.y = m_seedVertices[i].position.y;
	}

	DirectX::SimpleMath::Vector2 delta = maxima-minima;
	float maxDelta = std::max(delta.x, delta.y);

	// NB: In the case of a point, scaling won't matter
	if (maxDelta > 0.0f)
		m_scale /= maxDelta;

	// STEP 3: Rescale... 
	float xBorder = anchoring.x*(1.0f-delta.x/maxDelta);
	float yBorder = anchoring.y*(1.0f-delta.y/maxDelta);

	for (int i = 0; i < m_seedVertices.size(); i++)
	{
		m_seedVertices[i].position.x = xBorder+(m_seedVertices[i].position.x-minima.x)/maxDelta;
		m_seedVertices[i].position.y = yBorder+(m_seedVertices[i].position.y-minima.y)/maxDelta;

		m_seedVertices[i].depth *= m_scale;
		m_depth = std::max(m_seedVertices[i].depth, m_depth);
	}

	// STEP 4: Instantaneously 'update' m_treeVertices...
	UpdateTree(0.0f, 0.0f);
}

bool LSystem::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	DirectX::SimpleMath::Vector3 normal, tangent, binormal;
	float weight = 0.0f;

	m_vertexCount = std::max((int)(4*m_treeVertices.size()), 3); // NB: Use 16-gon to approximate a circle at joints?
	m_indexCount = std::max((int)(12*m_treeVertices.size()), 3);

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
	int index = 0;

	TreeVertexType treeVertex, parentVertex;
	DirectX::SimpleMath::Vector3 orthogonal;
	for (int i = 0; i < m_treeVertices.size(); i++)
	{
		treeVertex = m_treeVertices[i];
		parentVertex = m_treeVertices[treeVertex.parent];

		DirectX::SimpleMath::Vector3::Transform(treeVertex.position-parentVertex.position,DirectX::SimpleMath::Matrix::CreateRotationZ(DirectX::XM_PIDIV2),orthogonal);
		orthogonal.Normalize();

		vertices[4*i].position = parentVertex.position+parentVertex.width*orthogonal;
		vertices[4*i+1].position = parentVertex.position-parentVertex.width*orthogonal;
		vertices[4*i+2].position = treeVertex.position-treeVertex.width*orthogonal;
		vertices[4*i+3].position = treeVertex.position+treeVertex.width*orthogonal;

		indices[12*i] = 4*treeVertex.parent+2;
		indices[12*i+1] = 4*treeVertex.parent+3;
		indices[12*i+2] = 4*i;

		indices[12*i+3] = 4*treeVertex.parent+2;
		indices[12*i+4] = 4*i;
		indices[12*i+5] = 4*i+1;

		indices[12*i+6] = 4*i;
		indices[12*i+7] = 4*i+1;
		indices[12*i+8] = 4*i+2;

		indices[12*i+9] = 4*i;
		indices[12*i+10] = 4*i+2;
		indices[12*i+11] = 4*i+3;
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

void LSystem::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	return;
}

void LSystem::RenderBuffers(ID3D11DeviceContext* deviceContext)
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

void LSystem::Shutdown()
{
	ShutdownBuffers();

	return;
}

void LSystem::ShutdownBuffers()
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

void LSystem::Update(ID3D11Device* device, float deltaTime, float deltaIntensity)
{
	if (m_intensity == std::max(0.0f, std::min(m_intensity+deltaIntensity, 1.0f)))
		return;

	// STEP 1: Clear buffers...
	ShutdownBuffers();

	// STEP 2: Update tree topology using new parameters...
	UpdateTree(deltaTime, deltaIntensity);

	// STEP 3: Re-initialise buffers...
	InitializeBuffers(device);
}

void LSystem::UpdateTree(float deltaTime, float deltaIntensity)
{
	std::srand(m_seed);
	m_time += deltaTime;
	m_intensity += deltaIntensity;
	m_intensity = std::max(0.0f, std::min(m_intensity, 1.0f));

	m_treeVertices = std::vector<TreeVertexType>();

	m_treeVertices.push_back(TreeVertexType());
	m_treeVertices[0].parent = 0;
	m_treeVertices[0].transform = DirectX::SimpleMath::Matrix::CreateRotationZ(m_rotation)*DirectX::SimpleMath::Matrix::CreateTranslation(m_seedVertices[0].position); // NB: Assumes we've initialised m_seedVertices!
	DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), m_treeVertices[0].transform, m_treeVertices[0].position);

	int parentIndex = 0;
	std::vector<int> parentIndices = std::vector<int>();

	DirectX::SimpleMath::Matrix localTransform = DirectX::SimpleMath::Matrix::Identity;

	// STEP 1: Create branching structure...
	int childIndex = 1;
	float staticRotation, periodicRotation;
	float staticLength, periodicLength, lengthScale; 
	float staticWidth, periodicWidth, widthScale;
	float period, synchronisation, oscillation;
	for each (LModuleType LModule in m_sentence)
	{
		if (LModule.letter == "[")
		{
			parentIndices.push_back(parentIndex);
		}
		else if (LModule.letter == "]" && parentIndices.size() > 0)
		{
			parentIndex = parentIndices[parentIndices.size()-1];
			parentIndices.pop_back();
		}
		else
		{
			/* ----------------------------------------------------------------------- */
			/* NB: This is where the 'procedural animation' of L-systems is handled... */
			/* Very basic, but could be expanded upon...                               */

			period = LModule.period + GetRNGRange(0.0f, std::max(LModule.aperiodicity, 0.0f));
			synchronisation = LModule.synchronisation + GetRNGRange(0.0f, LModule.asynchronicity);
			oscillation = (period > 0.0f) ? cos(DirectX::XM_2PI * (m_time / period) + synchronisation) : 0.0f;
			
			staticRotation = LModule.staticRotation+GetRNGRange()*LModule.randomStaticRotation;
			periodicRotation = oscillation*(LModule.periodicRotation+GetRNGRange()*LModule.randomPeriodicRotation);
			localTransform = DirectX::SimpleMath::Matrix::CreateRotationZ(staticRotation+periodicRotation) * localTransform;

			if (LModule.staticLength == 0.0f)
				continue;

			lengthScale = (m_seedVertices[childIndex].depth > 0.0f) ? m_scale * std::max(0.0f,std::min((1.0f-m_depth/m_seedVertices[childIndex].depth)+(m_depth/m_seedVertices[childIndex].depth)*m_intensity, 1.0f)) : 0.0f;
			widthScale = m_scale * m_intensity;

			period = LModule.period + GetRNGRange(0.0f, std::max(LModule.aperiodicity, 0.0f));
			synchronisation = LModule.synchronisation + GetRNGRange(0.0f, LModule.asynchronicity);
			oscillation = (period > 0.0f) ? cos(DirectX::XM_2PI * (m_time / period) + synchronisation) : 0.0f;

			staticLength = LModule.staticLength+GetRNGRange()*LModule.randomStaticLength;
			periodicLength = oscillation*(LModule.periodicLength+GetRNGRange()*LModule.randomPeriodicLength);
			localTransform = DirectX::SimpleMath::Matrix::CreateTranslation(lengthScale*(staticLength+periodicLength), 0.0f, 0.0f) * localTransform;

			period = LModule.period + GetRNGRange(0.0f, std::max(LModule.aperiodicity, 0.0f));
			synchronisation = LModule.synchronisation + GetRNGRange(0.0f, LModule.asynchronicity);
			oscillation = (period > 0.0f) ? cos(DirectX::XM_2PI * (m_time / period) + synchronisation) : 0.0f;

			staticWidth = LModule.staticWidth+GetRNGRange()*LModule.randomStaticWidth;
			periodicWidth = oscillation*(LModule.periodicWidth+GetRNGRange()*LModule.randomPeriodicWidth);

			/* ----------------------------------------------------------------------- */

			m_treeVertices.push_back(TreeVertexType());
			m_treeVertices[childIndex].parent = parentIndex;
			m_treeVertices[childIndex].transform = localTransform * m_treeVertices[parentIndex].transform;
			DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f), m_treeVertices[childIndex].transform, m_treeVertices[childIndex].position);

			m_treeVertices[childIndex].width = widthScale*(staticWidth+periodicWidth);
			m_treeVertices[childIndex].degree = 1;

			m_treeVertices[parentIndex].width = std::max(m_treeVertices[childIndex].width, m_treeVertices[parentIndex].width);
			m_treeVertices[parentIndex].degree++;

			parentIndex = childIndex;
			childIndex = m_treeVertices.size();
			localTransform = DirectX::SimpleMath::Matrix::Identity;
		}
	}
}

void LSystem::AddProductionRule(std::string letter, LProductionRuleType productionRule)
{
	if (!m_productionRules.count(letter))
		m_productionRules.insert({ letter, std::vector<LProductionRuleType>{ productionRule } });
	else
		m_productionRules[letter].push_back(productionRule);
}

LSystem::LProductionRuleType LSystem::GetProductionRule(std::string letter)
{
	if (!m_productionRules.count(letter))
	{
		LProductionRuleType identity;
		identity.productions.push_back([](LSystem::LModuleType LModule) { return LModule; });
		identity.weight = 1.0f;
		return identity;
	}

	// FIXME: Add stochastic components here... 
	float totalWeight = 0.0f;
	for each (LProductionRuleType productionRule in m_productionRules[letter])
	{
		totalWeight += productionRule.weight;
	}

	int index = 0;
	float weight = GetRNGRange(0.0f, totalWeight);
	float summedWeight = 0.0f;
	for each (LProductionRuleType productionRule in m_productionRules[letter])
	{
		if (weight > summedWeight+productionRule.weight)
		{
			summedWeight += productionRule.weight;
			index++;
		}
		else
		{
			break;
		}
	}

	return m_productionRules[letter][index];
}

float LSystem::GetRNGRange(float a, float b)
{
	// FIXME: Rework this around simplex noise in the future?
	return a+(b-a)*std::rand()/RAND_MAX;
}

float* LSystem::GetIntensity()
{
	return &m_intensity;
}