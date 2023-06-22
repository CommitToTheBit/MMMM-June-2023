//
// Game.h
//
#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "modelclass.h"
#include "Light.h"
#include "Input.h"
#include "RenderTexture.h"

#include "Board.h"

#include "LDragonCurve.h"
#include "LSphinxTiling.h"
#include "LPenroseP3.h"
#include "LDeterministicBloodVessel.h"
#include "LBloodVessel.h"

#include "StoryEngine.h"

#include "Screen.h"

#include "Camera.h"
#include "EnvironmentCamera.h"

#include "Shader.h"
#include "PheremoneShader.h"

#include <chrono>

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
#ifdef DXTK_AUDIO
    void NewAudioDevice();
#endif

    // Properties
    void GetDefaultSize( int& width, int& height ) const;
	
private:

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	}; 

    void Update(DX::StepTimer const& timer);

    void Render();

    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    int SetupGUI(std::string text = "", std::vector<std::string> choices = std::vector<std::string>());

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;
    int                                     m_width, m_height;
    float                                   m_aspectRatio;
    float                                   m_fov;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    float                                   m_time;
    bool                                    m_preRendered;

	//input manager. 
	Input									m_input;
	InputCommands							m_gameInputCommands;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;	
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

	// Scene Objects
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;
	std::unique_ptr<DirectX::GeometricPrimitive>                            m_testmodel;

	// lights
	Light																	m_Light;
    DirectX::SimpleMath::Vector4                                            m_Ambience;

	// Cameras
	Camera																	m_Camera;
    EnvironmentCamera                                                       m_environmentCamera;

	// textures
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_normalMap;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_ComputeTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_ComputePheremones;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_ComputeAlpha;

	// Shaders
    Shader                                                                  m_NeutralShader;
    Shader                                                                  m_CubeShader;
    Shader                                                                  m_SphereShader;
    Shader                                                                  m_HorizontalTorusShader;
    Shader                                                                  m_VerticalTorusShader;

    // Compute Shaders
    PheremoneShader::SpeciesBuffer*                                         m_SlimeSpecies;
    PheremoneShader                                                         m_SlimeAgentsShader;
    PheremoneShader                                                         m_SlimePheremoneShader;

    float                                                                   m_SlimeDivergence;

    // Render-to-Textures
    Screen                                                                  m_Screen;
    Shader                                                                  m_ScreenShader;

    RenderTexture*                                                          m_PhysicalRenderPass;
    RenderTexture*                                                          m_VeinsRenderPass;

    // Models
    MarchingCubes                                                           m_Cube;
    MarchingCubes                                                           m_Sphere;
    MarchingCubes                                                           m_Torus;

    // L-Systems
    int                                                                     m_BloodVesselCount;
    std::vector<LBloodVessel>                                               m_BloodVessels;

    // GUI
    ImFont*                                                                 m_defaultFont;
    ImFont*                                                                 m_choiceFont;

#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif
    

#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;
};