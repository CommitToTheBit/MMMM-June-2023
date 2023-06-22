#pragma once

#include "DeviceResources.h"
#include "Light.h"

//Class from which we create all shader objects used by the framework
//This single class can be expanded to accomodate shaders of all different types with different parameters
class Shader
{
public:
	Shader();
	~Shader();

	//we could extend this to load in only a vertex shader, only a pixel shader etc.  or specialised init for Geometry or domain shader. 
	//All the methods here simply create new versions corresponding to your needs
	bool InitShader(ID3D11Device* device, WCHAR* vsFilename, WCHAR* psFilename); //Loads the Vert / pixel Shader pair
	void EnableShader(ID3D11DeviceContext* context);

	// (Modular) Add-ons...
	bool InitMatrixBuffer(ID3D11Device* device);		// 0
	bool InitTimeBuffer(ID3D11Device* device);			// 1
	bool InitAlphaBuffer(ID3D11Device* device);			// 2
	bool InitLightBuffer(ID3D11Device* device);			// 3
	bool InitAspectRatioBuffer(ID3D11Device* device);	// 4
	bool InitStressBuffer(ID3D11Device* device);		// 5

	// (Modular) Add-ons...
	bool SetMatrixBuffer(ID3D11DeviceContext* context, DirectX::SimpleMath::Matrix* world, DirectX::SimpleMath::Matrix* view, DirectX::SimpleMath::Matrix* projection, bool culling);
	bool SetTimeBuffer(ID3D11DeviceContext* context, float time);
	bool SetAlphaBuffer(ID3D11DeviceContext* context, float alpha);
	bool SetLightBuffer(ID3D11DeviceContext* context, Light* light);
	bool SetAspectRatioBuffer(ID3D11DeviceContext* context, float aspectRatio);
	bool SetStressBuffer(ID3D11DeviceContext* context, float stress);

	bool SetShaderTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* texture, int vsStartSlot, int psStartSlot);

protected:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>								m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>								m_pixelShader;
	ID3D11InputLayout*														m_layout;

	// Matrix Buffer
	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
		bool culling;
		DirectX::SimpleMath::Vector3 padding;
	};
	ID3D11Buffer*															m_matrixBuffer;

	// Time Buffer
	struct TimeBufferType
	{
		float time;
		DirectX::SimpleMath::Vector3 padding;
	};
	ID3D11Buffer*															m_timeBuffer;

	// Alpha Buffer
	struct AlphaBufferType
	{
		float alpha;
		DirectX::SimpleMath::Vector3 padding;
	};
	ID3D11Buffer*															m_alphaBuffer;

	// Light Buffer(s)
	struct LightBufferType
	{
		DirectX::SimpleMath::Vector4 ambient;
		DirectX::SimpleMath::Vector4 diffuse;
		DirectX::SimpleMath::Vector3 position;
		float strength;
	};
	ID3D11Buffer*															m_lightBuffer;

	// Aspect Ratio Buffer
	struct AspectRatioBufferType
	{
		float aspectRatio;
		DirectX::SimpleMath::Vector3 padding;
	};
	ID3D11Buffer*															m_aspectRatioBuffer;

	// Stress Buffer
	struct StressBufferType
	{
		float stress;
		DirectX::SimpleMath::Vector3 padding;
	};
	ID3D11Buffer*															m_stressBuffer;


	// Sampler
	ID3D11SamplerState*														m_sampleState;
};