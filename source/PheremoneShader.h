#pragma once

#include "DeviceResources.h"

class PheremoneShader
{
public:
	struct SlimeBuffer
	{
		int species;									// NB: Number of species...
		int cells;										// NB: Number of agents across all species...
		float speed;									// NB: Global speed modifier (for performance)...
		float padding;
	};

	struct SpeciesBuffer
	{
		DirectX::SimpleMath::Vector4 colour;			// NB: Colour entered into final texture...

		DirectX::SimpleMath::Vector3 sensorMask;		// NB: Colour of pheremone emitted...
		DirectX::SimpleMath::Vector3 sensorAttraction;	// NB: Allows asymmetric attraction/repulsion...

		float speed;									// NB: ...
		float angularVelocity;							// NB: ...

		float sensorDistance;
		float sensorAngle;
		float sensorPixelRadius;
#
		float pheremoneDispersion;						// NB: Rate at which...

		SpeciesBuffer();
		~SpeciesBuffer();
	};

	struct CellBuffer
	{
		int species;
		DirectX::SimpleMath::Vector2 position;
		float angle;

		CellBuffer();
		~CellBuffer();
	};

	struct TimeBuffer
	{
		float time;
		float delta;
		DirectX::SimpleMath::Vector2 padding;
	};

	struct MouseBuffer
	{
		DirectX::SimpleMath::Vector2 position;

		bool lmbPressed, lmbHeld, lmbReleased;
		DirectX::SimpleMath::Vector2 lmbPressPosition;
		float lmbPressTime, lmbPressDelta;				// NB: Total time held, time held...

		bool rmbPressed, rmbHeld, rmbReleased;
		DirectX::SimpleMath::Vector2 rmbPressPosition;
		float rmbPressTime, rmbPressDelta;				// NB: Total time held, time held...
	};

	struct TextureBuffer
	{
		int width, height;
		DirectX::SimpleMath::Vector2 padding;
	};

public:
	PheremoneShader();
	~PheremoneShader();

	bool InitShader(ID3D11Device* device, WCHAR* csFilename);
	void EnableShader(ID3D11DeviceContext* context);
	void DisableShader(ID3D11DeviceContext* context);

	// SLOT b0
	bool InitSlimeBuffer(ID3D11Device* device, UINT species, UINT cells);
	void SetSlimeBuffer(ID3D11DeviceContext* context, float speed);

	// SLOT t0 / u0
	bool InitSpeciesBuffer(ID3D11Device* device);
	void SetSpeciesBuffer(ID3D11DeviceContext* context, SpeciesBuffer* species);
	SpeciesBuffer* GetSpeciesBuffer(ID3D11DeviceContext* context);

	// SLOT t1 / u1
	bool InitCellBuffer(ID3D11Device* device);
	void SetCellBuffer(ID3D11DeviceContext* context, CellBuffer* cells);
	CellBuffer* GetCellBuffer(ID3D11DeviceContext* context);

	// SLOT b1
	bool InitTimeBuffer(ID3D11Device* device);
	void SetTimeBuffer(ID3D11DeviceContext* context, float time, float delta);

	// SLOT b2
	bool InitMouseBuffer(ID3D11Device* device);
	void SetMouseBuffer(ID3D11DeviceContext* context, MouseBuffer* mouse);

	// SLOT b3
	bool InitTextureBuffer(ID3D11Device* device, UINT width, UINT height);
	void SetTextureBuffer(ID3D11DeviceContext* context);

	// SLOT t2 / u2
	bool InitTexture(ID3D11Device* device);
	void SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* texture);
	ID3D11ShaderResourceView* GetTexture(ID3D11DeviceContext* context);
	void ClearTexture(ID3D11DeviceContext* context);

	// SLOT t3 / u3
	bool InitPheremoneMask(ID3D11Device* device);
	void SetPheremoneMask(ID3D11DeviceContext* context, ID3D11ShaderResourceView* pheremoneMask);
	ID3D11ShaderResourceView* GetPheremoneMask(ID3D11DeviceContext* context);
	void ClearPheremoneMask(ID3D11DeviceContext* context);

	// SLOT t4 / u4
	bool InitSpeciesMaskA(ID3D11Device* device);
	void SetSpeciesMaskA(ID3D11DeviceContext* context, ID3D11ShaderResourceView* speciesMaskA);
	ID3D11ShaderResourceView* GetSpeciesMaskA(ID3D11DeviceContext* context);
	void ClearSpeciesMaskA(ID3D11DeviceContext* context);

	// SLOT t5 / u5
	bool InitSpeciesMaskB(ID3D11Device* device);
	void SetSpeciesMaskB(ID3D11DeviceContext* context, ID3D11ShaderResourceView* speciesMaskB);
	ID3D11ShaderResourceView* GetSpeciesMaskB(ID3D11DeviceContext* context);
	void ClearSpeciesMaskB(ID3D11DeviceContext* context);

	// SLOT t6 / u6
	bool InitSpeciesMaskC(ID3D11Device* device);
	void SetSpeciesMaskC(ID3D11DeviceContext* context, ID3D11ShaderResourceView* speciesMaskC);
	ID3D11ShaderResourceView* GetSpeciesMaskC(ID3D11DeviceContext* context);
	void ClearSpeciesMaskC(ID3D11DeviceContext* context);

	// SLOT t7 / u7
	bool InitFoodMask(ID3D11Device* device);
	void SetFoodMask(ID3D11DeviceContext* context, ID3D11ShaderResourceView* speciesMaskC);
	ID3D11ShaderResourceView* GetFoodMask(ID3D11DeviceContext* context);
	void ClearFoodMask(ID3D11DeviceContext* context);

private: // FIXME: Just need to write these manually...
	bool InitTextureSlot(ID3D11Device* device, ID3D11ShaderResourceView** setSRV, ID3D11ShaderResourceView** getSRV, ID3D11UnorderedAccessView** getUAV);
	void SetTextureSlot(ID3D11DeviceContext* context, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView** srv, ID3D11UnorderedAccessView** uav, int slot);
	void ClearTextureSlot(ID3D11DeviceContext* context, ID3D11UnorderedAccessView* uav);

public:
	int m_species, m_cells;
	int m_width, m_height; // NB: Include aspect ratio, for efficiency?

protected:
	//protected:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_pheremoneShader;

	// SLOT b0
	ID3D11Buffer* m_slimeBuffer;

	// SLOT t0 / u0
	ID3D11Buffer* m_setSpeciesBuffer;
	ID3D11ShaderResourceView* m_setSpeciesSRV;

	ID3D11Buffer* m_getSpeciesBuffer;
	ID3D11Buffer* m_getReadableSpeciesBuffer;
	ID3D11UnorderedAccessView* m_getSpeciesUAV;

	// SLOT t1 / u1
	ID3D11Buffer* m_setCellBuffer;
	ID3D11ShaderResourceView* m_setCellSRV;

	ID3D11Buffer* m_getCellBuffer;
	ID3D11Buffer* m_getReadableCellBuffer;
	ID3D11UnorderedAccessView* m_getCellUAV;

	// SLOT b2
	ID3D11Buffer* m_timeBuffer;

	// SLOT b3
	ID3D11Buffer* m_mouseBuffer;

	// SLOT b4
	ID3D11Buffer* m_textureBuffer;

	// SLOT t2 / u2
	ID3D11ShaderResourceView* m_setTextureSRV;

	ID3D11ShaderResourceView* m_getTextureSRV;
	ID3D11UnorderedAccessView* m_getTextureUAV;

	// SLOT t2 / u2
	ID3D11ShaderResourceView* m_setPheremoneMaskSRV;

	ID3D11ShaderResourceView* m_getPheremoneMaskSRV;
	ID3D11UnorderedAccessView* m_getPheremoneMaskUAV;

	// SLOT t3 / u3
	ID3D11ShaderResourceView* m_setSpeciesMaskASRV;

	ID3D11ShaderResourceView* m_getSpeciesMaskASRV;
	ID3D11UnorderedAccessView* m_getSpeciesMaskAUAV;

	// SLOT t4 / u4
	ID3D11ShaderResourceView* m_setSpeciesMaskBSRV;

	ID3D11ShaderResourceView* m_getSpeciesMaskBSRV;
	ID3D11UnorderedAccessView* m_getSpeciesMaskBUAV;

	// SLOT t5 / u5
	ID3D11ShaderResourceView* m_setSpeciesMaskCSRV;

	ID3D11ShaderResourceView* m_getSpeciesMaskCSRV;
	ID3D11UnorderedAccessView* m_getSpeciesMaskCUAV;

	// SLOT t5 / u5
	ID3D11ShaderResourceView* m_setFoodMaskSRV;

	ID3D11ShaderResourceView* m_getFoodMaskSRV;
	ID3D11UnorderedAccessView* m_getFoodMaskUAV;
};

