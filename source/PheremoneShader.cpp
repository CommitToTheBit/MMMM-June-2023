#include "pch.h"
#include "PheremoneShader.h"

PheremoneShader::SpeciesBuffer::SpeciesBuffer()
{

}

PheremoneShader::SpeciesBuffer::~SpeciesBuffer()
{

}

PheremoneShader::CellBuffer::CellBuffer()
{

}

PheremoneShader::CellBuffer::~CellBuffer()
{

}

PheremoneShader::PheremoneShader()
{

}

PheremoneShader::~PheremoneShader()
{

}

bool PheremoneShader::InitShader(ID3D11Device* device, WCHAR* csFilename)
{
	//LOAD SHADER: COMPUTE
	auto PheremoneShaderBuffer = DX::ReadData(csFilename);
	HRESULT result = device->CreateComputeShader(PheremoneShaderBuffer.data(), PheremoneShaderBuffer.size(), NULL, &m_pheremoneShader);
	if (result != S_OK)
	{
		//if loading failed.  
		return false;
	}

	return true;
}

void PheremoneShader::EnableShader(ID3D11DeviceContext* context)
{
	context->CSSetShader(m_pheremoneShader.Get(), 0, 0); //turn on compute shader
}

void PheremoneShader::DisableShader(ID3D11DeviceContext* context)
{
	ID3D11UnorderedAccessView* NullUAV = nullptr;
	context->CSSetUnorderedAccessViews(0, 1, &NullUAV, nullptr);
	context->CSSetUnorderedAccessViews(1, 1, &NullUAV, nullptr);
	context->CSSetUnorderedAccessViews(2, 1, &NullUAV, nullptr);
	context->CSSetUnorderedAccessViews(3, 1, &NullUAV, nullptr);
	context->CSSetUnorderedAccessViews(4, 1, &NullUAV, nullptr);
	context->CSSetUnorderedAccessViews(5, 1, &NullUAV, nullptr);
	context->CSSetUnorderedAccessViews(6, 1, &NullUAV, nullptr);
	context->CSSetUnorderedAccessViews(7, 1, &NullUAV, nullptr);

	context->CSSetShader(nullptr, 0, 0);
}

bool PheremoneShader::InitSlimeBuffer(ID3D11Device* device, UINT species, UINT cells)
{
	m_species = species;
	m_cells = cells;

	// Initialise slot b0...
	D3D11_BUFFER_DESC slimeBufferDesc;
	slimeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	slimeBufferDesc.ByteWidth = sizeof(SlimeBuffer);
	slimeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	slimeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	slimeBufferDesc.MiscFlags = 0;
	slimeBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&slimeBufferDesc, NULL, &m_slimeBuffer);

	return true;
}

void PheremoneShader::SetSlimeBuffer(ID3D11DeviceContext* context, float speed)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_slimeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	SlimeBuffer* slimePtr = (SlimeBuffer*)mappedResource.pData;
	slimePtr->species = m_species;
	slimePtr->cells = m_cells;
	slimePtr->speed = speed;
	slimePtr->padding = 0.0f;
	context->Unmap(m_slimeBuffer, 0);

	context->CSSetConstantBuffers(0, 1, &m_slimeBuffer);
}

bool PheremoneShader::InitSpeciesBuffer(ID3D11Device* device)
{
	// Initialise slot t0...
	D3D11_BUFFER_DESC setSpeciesBufferDesc;
	setSpeciesBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	setSpeciesBufferDesc.ByteWidth = m_species * sizeof(SpeciesBuffer);
	setSpeciesBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	setSpeciesBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	setSpeciesBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	setSpeciesBufferDesc.StructureByteStride = sizeof(SpeciesBuffer);

	device->CreateBuffer(&setSpeciesBufferDesc, NULL, &m_setSpeciesBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC setSpeciesSRVDesc;
	setSpeciesSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	setSpeciesSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	setSpeciesSRVDesc.BufferEx.FirstElement = 0;
	setSpeciesSRVDesc.BufferEx.Flags = 0;
	setSpeciesSRVDesc.BufferEx.NumElements = m_species;

	device->CreateShaderResourceView(m_setSpeciesBuffer, &setSpeciesSRVDesc, &m_setSpeciesSRV);

	// Initialise slot u0...
	D3D11_BUFFER_DESC getSpeciesBufferDesc;
	getSpeciesBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	getSpeciesBufferDesc.ByteWidth = m_species * sizeof(SpeciesBuffer);
	getSpeciesBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	getSpeciesBufferDesc.CPUAccessFlags = 0;
	getSpeciesBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	getSpeciesBufferDesc.StructureByteStride = sizeof(SpeciesBuffer);

	device->CreateBuffer(&getSpeciesBufferDesc, NULL, &m_getSpeciesBuffer);

	getSpeciesBufferDesc.Usage = D3D11_USAGE_STAGING;
	getSpeciesBufferDesc.BindFlags = 0;
	getSpeciesBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	device->CreateBuffer(&getSpeciesBufferDesc, NULL, &m_getReadableSpeciesBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC getSpeciesUAVDesc;
	getSpeciesUAVDesc.Buffer.FirstElement = 0;
	getSpeciesUAVDesc.Buffer.Flags = 0;
	getSpeciesUAVDesc.Buffer.NumElements = m_species;
	getSpeciesUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	getSpeciesUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	device->CreateUnorderedAccessView(m_getSpeciesBuffer, &getSpeciesUAVDesc, &m_getSpeciesUAV);

	return true;
}

void PheremoneShader::SetSpeciesBuffer(ID3D11DeviceContext* context, SpeciesBuffer* species)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_setSpeciesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	SpeciesBuffer* setSpeciesPtr = (SpeciesBuffer*)mappedResource.pData;
	for (int i = 0; i < m_species; i++)
	{
		setSpeciesPtr[i].colour = species[i].colour;
		setSpeciesPtr[i].sensorMask = species[i].sensorMask;
		setSpeciesPtr[i].sensorAttraction = species[i].sensorAttraction;
		setSpeciesPtr[i].speed = species[i].speed;
		setSpeciesPtr[i].angularVelocity = species[i].angularVelocity;
		setSpeciesPtr[i].sensorDistance = species[i].sensorDistance;
		setSpeciesPtr[i].sensorAngle = species[i].sensorAngle;
		setSpeciesPtr[i].sensorPixelRadius = species[i].sensorPixelRadius;
		setSpeciesPtr[i].pheremoneDispersion = species[i].pheremoneDispersion;
	}
	context->Unmap(m_setSpeciesBuffer, 0);

	context->CSSetShaderResources(0, 1, &m_setSpeciesSRV);
	context->CSSetUnorderedAccessViews(0, 1, &m_getSpeciesUAV, 0);
}

PheremoneShader::SpeciesBuffer* PheremoneShader::GetSpeciesBuffer(ID3D11DeviceContext* context)
{
	context->CopyResource(m_getReadableSpeciesBuffer, m_getSpeciesBuffer);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_getReadableSpeciesBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	SpeciesBuffer* speciesBuffer = reinterpret_cast<SpeciesBuffer*>(mappedResource.pData);
	context->Unmap(m_getReadableSpeciesBuffer, 0);

	return speciesBuffer;
}

bool PheremoneShader::InitCellBuffer(ID3D11Device* device)
{
	// Initialise slot t1...
	D3D11_BUFFER_DESC setCellBufferDesc;
	setCellBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	setCellBufferDesc.ByteWidth = m_cells * sizeof(CellBuffer);
	setCellBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	setCellBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	setCellBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	setCellBufferDesc.StructureByteStride = sizeof(CellBuffer);

	device->CreateBuffer(&setCellBufferDesc, NULL, &m_setCellBuffer);

	D3D11_SHADER_RESOURCE_VIEW_DESC setCellSRVDesc;
	setCellSRVDesc.Format = DXGI_FORMAT_UNKNOWN;
	setCellSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	setCellSRVDesc.BufferEx.FirstElement = 0;
	setCellSRVDesc.BufferEx.Flags = 0;
	setCellSRVDesc.BufferEx.NumElements = m_cells;

	device->CreateShaderResourceView(m_setCellBuffer, &setCellSRVDesc, &m_setCellSRV);

	// Initialise slot u1...
	D3D11_BUFFER_DESC getCellBufferDesc;
	getCellBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	getCellBufferDesc.ByteWidth = m_cells * sizeof(CellBuffer);
	getCellBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	getCellBufferDesc.CPUAccessFlags = 0;
	getCellBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	getCellBufferDesc.StructureByteStride = sizeof(CellBuffer);

	device->CreateBuffer(&getCellBufferDesc, NULL, &m_getCellBuffer);

	getCellBufferDesc.Usage = D3D11_USAGE_STAGING;
	getCellBufferDesc.BindFlags = 0;
	getCellBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	device->CreateBuffer(&getCellBufferDesc, NULL, &m_getReadableCellBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC getCellUAVDesc;
	getCellUAVDesc.Buffer.FirstElement = 0;
	getCellUAVDesc.Buffer.Flags = 0;
	getCellUAVDesc.Buffer.NumElements = m_cells;
	getCellUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
	getCellUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

	device->CreateUnorderedAccessView(m_getCellBuffer, &getCellUAVDesc, &m_getCellUAV);

	return true;
}

void PheremoneShader::SetCellBuffer(ID3D11DeviceContext* context, CellBuffer* cells)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_setCellBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CellBuffer* setCellPtr = (CellBuffer*)mappedResource.pData;
	for (int i = 0; i < m_cells; i++)
	{
		setCellPtr[i].species = cells[i].species;
		setCellPtr[i].position = cells[i].position;
		setCellPtr[i].angle = cells[i].angle;
	}
	context->Unmap(m_setCellBuffer, 0);

	context->CSSetShaderResources(1, 1, &m_setCellSRV);
	context->CSSetUnorderedAccessViews(1, 1, &m_getCellUAV, 0); // CHECKME: Can this cause a crash?
}

PheremoneShader::CellBuffer* PheremoneShader::GetCellBuffer(ID3D11DeviceContext* context)
{
	context->CopyResource(m_getReadableCellBuffer, m_getCellBuffer);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_getReadableCellBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	CellBuffer* cellBuffer = reinterpret_cast<CellBuffer*>(mappedResource.pData);
	context->Unmap(m_getReadableCellBuffer, 0);

	return cellBuffer;
}

bool PheremoneShader::InitTimeBuffer(ID3D11Device* device)
{
	// Initialise slot b1...
	D3D11_BUFFER_DESC timeBufferDesc;
	timeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	timeBufferDesc.ByteWidth = sizeof(TimeBuffer);
	timeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timeBufferDesc.MiscFlags = 0;
	timeBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&timeBufferDesc, NULL, &m_timeBuffer);

	return true;
}

void PheremoneShader::SetTimeBuffer(ID3D11DeviceContext* context, float time, float delta)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_timeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TimeBuffer* timePtr = (TimeBuffer*)mappedResource.pData;
	timePtr->time = time;
	timePtr->delta = delta;
	timePtr->padding = DirectX::SimpleMath::Vector2(0.0f, 0.0f);
	context->Unmap(m_timeBuffer, 0);

	context->CSSetConstantBuffers(1, 1, &m_timeBuffer);
}

bool PheremoneShader::InitMouseBuffer(ID3D11Device* device)
{
	// Initialise slot b1...
	D3D11_BUFFER_DESC mouseBufferDesc;
	mouseBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	mouseBufferDesc.ByteWidth = sizeof(MouseBuffer);
	mouseBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mouseBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mouseBufferDesc.MiscFlags = 0;
	mouseBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&mouseBufferDesc, NULL, &m_mouseBuffer);

	return true;
}

void PheremoneShader::SetMouseBuffer(ID3D11DeviceContext* context, MouseBuffer* mouse)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_mouseBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MouseBuffer* mousePtr = (MouseBuffer*)mappedResource.pData;
	mousePtr->position = mouse->position;
	mousePtr->lmbPressed = mouse->lmbPressed;
	mousePtr->lmbHeld = mouse->lmbHeld;
	mousePtr->lmbReleased = mouse->lmbReleased;
	mousePtr->lmbPressPosition = mouse->lmbPressPosition;
	mousePtr->lmbPressTime = mouse->lmbPressTime;
	mousePtr->lmbPressDelta = mouse->lmbPressDelta;
	mousePtr->rmbPressed = mouse->rmbPressed;
	mousePtr->rmbHeld = mouse->rmbHeld;
	mousePtr->rmbReleased = mouse->rmbReleased;
	mousePtr->rmbPressPosition = mouse->rmbPressPosition;
	mousePtr->rmbPressTime = mouse->rmbPressTime;
	mousePtr->rmbPressDelta = mouse->rmbPressDelta;
	context->Unmap(m_timeBuffer, 0);

	context->CSSetConstantBuffers(2, 1, &m_timeBuffer);
}

bool PheremoneShader::InitTextureBuffer(ID3D11Device* device, UINT width, UINT height)
{
	m_width = width;
	m_height = height;

	// Initialise slot b1...
	D3D11_BUFFER_DESC textureBufferDesc;
	textureBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureBufferDesc.ByteWidth = sizeof(TextureBuffer);
	textureBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	textureBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	textureBufferDesc.MiscFlags = 0;
	textureBufferDesc.StructureByteStride = 0;

	device->CreateBuffer(&textureBufferDesc, NULL, &m_textureBuffer);

	return true;
}

void PheremoneShader::SetTextureBuffer(ID3D11DeviceContext* context)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	context->Map(m_textureBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TextureBuffer* texturePtr = (TextureBuffer*)mappedResource.pData;
	texturePtr->width = m_width;
	texturePtr->height = m_height;
	texturePtr->padding = DirectX::SimpleMath::Vector2(0.0f, 0.0f);
	context->Unmap(m_textureBuffer, 0);

	context->CSSetConstantBuffers(3, 1, &m_textureBuffer);
}

bool PheremoneShader::InitTexture(ID3D11Device* device)
{
	return InitTextureSlot(device, &m_setTextureSRV, &m_getTextureSRV, &m_getTextureUAV);
}

void PheremoneShader::SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* texture)
{
	SetTextureSlot(context, texture, &m_setTextureSRV, &m_getTextureUAV, 2);
}

ID3D11ShaderResourceView* PheremoneShader::GetTexture(ID3D11DeviceContext* context)
{
	return m_getTextureSRV;
}

void PheremoneShader::ClearTexture(ID3D11DeviceContext* context)
{
	ClearTextureSlot(context, m_getTextureUAV);
}

bool PheremoneShader::InitPheremoneMask(ID3D11Device* device)
{
	return InitTextureSlot(device, &m_setPheremoneMaskSRV, &m_getPheremoneMaskSRV, &m_getPheremoneMaskUAV);
}

void PheremoneShader::SetPheremoneMask(ID3D11DeviceContext* context, ID3D11ShaderResourceView* pheremoneMask)
{
	SetTextureSlot(context, pheremoneMask, &m_setPheremoneMaskSRV, &m_getPheremoneMaskUAV, 3);
}

ID3D11ShaderResourceView* PheremoneShader::GetPheremoneMask(ID3D11DeviceContext* context)
{
	return m_getPheremoneMaskSRV;
}

void PheremoneShader::ClearPheremoneMask(ID3D11DeviceContext* context)
{
	ClearTextureSlot(context, m_getPheremoneMaskUAV);
}

bool PheremoneShader::InitSpeciesMaskA(ID3D11Device* device)
{
	return InitTextureSlot(device, &m_setSpeciesMaskASRV, &m_getSpeciesMaskASRV, &m_getSpeciesMaskAUAV);
}

void PheremoneShader::SetSpeciesMaskA(ID3D11DeviceContext* context, ID3D11ShaderResourceView* speciesMaskA)
{
	SetTextureSlot(context, speciesMaskA, &m_setSpeciesMaskASRV, &m_getSpeciesMaskAUAV, 4);
}

ID3D11ShaderResourceView* PheremoneShader::GetSpeciesMaskA(ID3D11DeviceContext* context)
{
	return m_getSpeciesMaskASRV;
}

void PheremoneShader::ClearSpeciesMaskA(ID3D11DeviceContext* context)
{
	ClearTextureSlot(context, m_getSpeciesMaskAUAV);

	context->ClearUnorderedAccessViewFloat(m_getSpeciesMaskAUAV, new FLOAT[4]{ 0.0f, 0.0f, 0.0f, 0.0f });
}

bool PheremoneShader::InitSpeciesMaskB(ID3D11Device* device)
{
	return InitTextureSlot(device, &m_setSpeciesMaskBSRV, &m_getSpeciesMaskBSRV, &m_getSpeciesMaskBUAV);
}

void PheremoneShader::SetSpeciesMaskB(ID3D11DeviceContext* context, ID3D11ShaderResourceView* speciesMaskB)
{
	SetTextureSlot(context, speciesMaskB, &m_setSpeciesMaskBSRV, &m_getSpeciesMaskBUAV, 5);
}

ID3D11ShaderResourceView* PheremoneShader::GetSpeciesMaskB(ID3D11DeviceContext* context)
{
	return m_getSpeciesMaskBSRV;
}

void PheremoneShader::ClearSpeciesMaskB(ID3D11DeviceContext* context)
{
	ClearTextureSlot(context, m_getSpeciesMaskBUAV);
}

bool PheremoneShader::InitSpeciesMaskC(ID3D11Device* device)
{
	return InitTextureSlot(device, &m_setSpeciesMaskCSRV, &m_getSpeciesMaskCSRV, &m_getSpeciesMaskCUAV);
}

void PheremoneShader::SetSpeciesMaskC(ID3D11DeviceContext* context, ID3D11ShaderResourceView* speciesMaskC)
{
	SetTextureSlot(context, speciesMaskC, &m_setSpeciesMaskCSRV, &m_getSpeciesMaskCUAV, 6);
}

ID3D11ShaderResourceView* PheremoneShader::GetSpeciesMaskC(ID3D11DeviceContext* context)
{
	return m_getSpeciesMaskCSRV;
}

void PheremoneShader::ClearSpeciesMaskC(ID3D11DeviceContext* context)
{
	ClearTextureSlot(context, m_getSpeciesMaskCUAV);
}

bool PheremoneShader::InitFoodMask(ID3D11Device* device)
{
	return InitTextureSlot(device, &m_setFoodMaskSRV, &m_getFoodMaskSRV, &m_getFoodMaskUAV);
}

void PheremoneShader::SetFoodMask(ID3D11DeviceContext* context, ID3D11ShaderResourceView* FoodMask)
{
	SetTextureSlot(context, FoodMask, &m_setFoodMaskSRV, &m_getFoodMaskUAV, 7);
}

ID3D11ShaderResourceView* PheremoneShader::GetFoodMask(ID3D11DeviceContext* context)
{
	return m_getFoodMaskSRV;
}

void PheremoneShader::ClearFoodMask(ID3D11DeviceContext* context)
{
	ClearTextureSlot(context, m_getFoodMaskUAV);
}

/* -------------------------------------------------------------------------------------------------------------------- */
/* This enclosed section has been copied from: Frank D. Luna (2012) Introduction to 3D Game Programming with DirectX 11 */

bool PheremoneShader::InitTextureSlot(ID3D11Device* device, ID3D11ShaderResourceView** setSRV, ID3D11ShaderResourceView** getSRV, ID3D11UnorderedAccessView** getUAV)
{
	// NB: Need to pass in ID3D11ShaderResourceView**, ... , to act on them!

	// Initialise slot t...
	D3D11_TEXTURE2D_DESC setTextureSlotDesc;
	setTextureSlotDesc.Width = m_width;
	setTextureSlotDesc.Height = m_height;
	setTextureSlotDesc.MipLevels = 1;
	setTextureSlotDesc.ArraySize = 1;
	setTextureSlotDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	setTextureSlotDesc.SampleDesc.Count = 1;
	setTextureSlotDesc.SampleDesc.Quality = 0;
	setTextureSlotDesc.Usage = D3D11_USAGE_DYNAMIC;
	setTextureSlotDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	setTextureSlotDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	setTextureSlotDesc.MiscFlags = 0;

	ID3D11Texture2D* setTextureSlot = 0;
	device->CreateTexture2D(&setTextureSlotDesc, 0, &setTextureSlot);

	D3D11_SHADER_RESOURCE_VIEW_DESC setTextureSRVDesc;
	setTextureSRVDesc.Format = setTextureSlotDesc.Format;
	setTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	setTextureSRVDesc.Texture2D.MostDetailedMip = 0;
	setTextureSRVDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(setTextureSlot, &setTextureSRVDesc, setSRV);

	// Initialise slot u...
	D3D11_TEXTURE2D_DESC getTextureSlotDesc;
	getTextureSlotDesc.Width = m_width;
	getTextureSlotDesc.Height = m_height;
	getTextureSlotDesc.MipLevels = 1;
	getTextureSlotDesc.ArraySize = 1;
	getTextureSlotDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	getTextureSlotDesc.SampleDesc.Count = 1;
	getTextureSlotDesc.SampleDesc.Quality = 0;
	getTextureSlotDesc.Usage = D3D11_USAGE_DEFAULT;
	getTextureSlotDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	getTextureSlotDesc.CPUAccessFlags = 0;
	getTextureSlotDesc.MiscFlags = 0;

	ID3D11Texture2D* getTextureSlot = 0;
	device->CreateTexture2D(&getTextureSlotDesc, 0, &getTextureSlot);

	D3D11_SHADER_RESOURCE_VIEW_DESC getTextureSRVDesc;
	getTextureSRVDesc.Format = getTextureSlotDesc.Format;
	getTextureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	getTextureSRVDesc.Texture2D.MostDetailedMip = 0;
	getTextureSRVDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(getTextureSlot, &getTextureSRVDesc, getSRV);

	D3D11_UNORDERED_ACCESS_VIEW_DESC getTextureUAVDesc;
	getTextureUAVDesc.Format = getTextureSlotDesc.Format;
	getTextureUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	getTextureUAVDesc.Texture2D.MipSlice = 0;

	device->CreateUnorderedAccessView(getTextureSlot, &getTextureUAVDesc, getUAV);

	setTextureSlot->Release();
	getTextureSlot->Release();

	return true;
}

void PheremoneShader::SetTextureSlot(ID3D11DeviceContext* context, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView** srv, ID3D11UnorderedAccessView** uav, int slot)
{
	ID3D11Resource* sourceTex, * destinationTex;
	texture->GetResource(&sourceTex);
	(*srv)->GetResource(&destinationTex);
	context->CopyResource(destinationTex, sourceTex);

	context->CSSetShaderResources(slot, 1, srv);
	context->CSSetUnorderedAccessViews(slot, 1, uav, NULL);
}

void PheremoneShader::ClearTextureSlot(ID3D11DeviceContext* context, ID3D11UnorderedAccessView* uav) // FIXME: Can we remove one *?
{
	context->ClearUnorderedAccessViewFloat(uav, new FLOAT[4]{ 0.0f, 0.0f, 0.0f, 0.0f }); // NB: Will this work for species, etc.? // new FLOAT[4]{ 0.0f, 0.0f, 0.0f, 0.0f });
}

/* -------------------------------------------------------------------------------------------------------------------- */