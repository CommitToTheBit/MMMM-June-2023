//
// Game.cpp
//

#include "pch.h"
#include "Game.h"


//toreorganise
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace ImGui;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	//setup imgui.  its up here cos we need the window handle too
	//pulled from imgui directx11 example
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); // FIXME: Is (void)io; necessary?
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);		//tie to our window
	ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());	//tie to directx

	m_defaultFont = io.Fonts->AddFontFromFileTTF("beneg___.ttf", 54); // NB: pt-to-px conversion: px = (4.0f/3.0f)*pt
	m_choiceFont = io.Fonts->AddFontFromFileTTF("beneg___.ttf", 42);

	ImGuiStyle& style = ImGui::GetStyle();
	style.ButtonTextAlign = ImVec2(0.0f, 0.5f);
	style.FrameRounding = 12.0f;
	style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.8f, 0.2f, 0.2f, 0.5f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.8f, 0.2f, 0.2f, 0.75f);

	//setup light
	m_Ambience = Vector4(0.15f, 0.15f, 0.15f, 1.0f);
	m_Light.setAmbientColour(m_Ambience.x, m_Ambience.y, m_Ambience.z, m_Ambience.w);
	m_Light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.setPosition(2.0f, 2.0f, 2.0f);
	m_Light.setDirection(1.0f, 1.0f, 1.0f);
	m_Light.setStrength(100.0f);

	// CAMERA:
	m_Camera.setPosition(Vector3(0.0f,0.0f, 0.0f));
	m_Camera.setRotation(Vector3(-90.0f, -180.0f, 0.0f));
	
#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"616516__justlaz__geiger_tick_low.wav");
    m_effect1 = m_soundEffect->CreateInstance();

    //m_effect1->Play(true); // DEBUG: Comment out to mute...
#endif
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	//Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	auto device = m_deviceResources->GetD3DDevice();

	m_time = m_timer.GetTotalSeconds();

	// CAMERA INPUTS:;
	Vector3 inputPosition = Vector3(0.0f, 0.0f, 0.0f);

	// STEP 1: Read camera translation inputs (from keyboard)
	if (m_gameInputCommands.forward)
		inputPosition.z += 1.0f;
	if (m_gameInputCommands.back)
		inputPosition.z -= 1.0f;
	if (m_gameInputCommands.right)
		inputPosition.x += 1.0f;
	if (m_gameInputCommands.left)
		inputPosition.x -= 1.0f;
	if (m_gameInputCommands.up)
		inputPosition.y += 1.0f;
	if (m_gameInputCommands.down)
		inputPosition.y -= 1.0f;

	if (inputPosition.x != 0.0f || inputPosition.y != 0.0f || inputPosition.z != 0.0f)
	{
		inputPosition.Normalize();
		inputPosition.x *= 0.8f;
		inputPosition.y *= 0.8f;

		// NB: forward/right directions are relative to the camera, but up remains relative to the space to avoid confusion...
		Vector3 deltaPosition = inputPosition.z*m_Camera.getForward() + inputPosition.x*m_Camera.getRight() + inputPosition.y*Vector3::UnitY;
		deltaPosition *= m_timer.GetElapsedSeconds()*m_Camera.getMoveSpeed();

		Vector3 position = m_Camera.getPosition()+deltaPosition;
		m_Camera.setPosition(position);
	}

	// STEP 2: Read camera rotation inputs (from mouse)
	Vector2 inputRotation = m_gameInputCommands.rotation;
	if (inputRotation.x != 0.0f || inputRotation.y != 0.0f)
	{
		Vector3 rotation = m_Camera.getRotation();
		inputRotation.x *= sin(XM_PIDIV4+0.5*XM_PI*(-rotation.x/180.0f));

		Vector3 deltaRotation = (-inputRotation.y)*Vector3::UnitX + (-inputRotation.x)*Vector3::UnitY;
		deltaRotation *= m_timer.GetElapsedSeconds()*m_Camera.getRotationSpeed();

		rotation += deltaRotation;
		rotation.x = std::min(-0.001f, std::max(rotation.x, -179.999f)); // NB: Prevents gimbal lock
		m_Camera.setRotation(rotation);
	}

	// STEP 3: Process inputs
	m_Camera.Update();

	m_view = m_Camera.getCameraMatrix();
	m_projection = m_Camera.getPerspective();
	m_world = Matrix::Identity;

	// ANT INPUTS
	if (m_gameInputCommands.clockwise)
		m_SlimeDivergence -= m_timer.GetElapsedSeconds() / 40.0f;
	else
		m_SlimeDivergence += m_timer.GetElapsedSeconds() / 80.0f;
	m_SlimeDivergence = std::max(-1.0f, std::min(m_SlimeDivergence, 1.0f));

	/*create our UI*/
	SetupGUI();

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }

#endif

	if (m_input.Quit())
	{
		ExitGame();
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

	//Set Rendering states. 
	context->OMSetBlendState(m_states->NonPremultiplied(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());

	// If m_time == 0.0, then render all static textures (once only!)
	if (!m_preRendered)
	{
		m_preRendered = true;
	}

	// COMPUTE SHADER RENDER
	//m_SlimeSpecies[0].colour = Vector4(0.75f + 0.25f * sin(XM_2PI * m_timer.GetTotalSeconds()), 0.75f - 0.25f * sin(XM_2PI * m_timer.GetTotalSeconds()), 1.0f, 1.0f);
	//m_SlimeSpecies[1].colour = Vector4(0.75f - 0.25f * sin(XM_2PI * m_timer.GetTotalSeconds()), 0.75f + 0.25f * sin(XM_2PI * m_timer.GetTotalSeconds()), 1.0f, 1.0f);

	m_SlimeSpecies[0].sensorMask = Vector3(0.5f+0.5f*m_SlimeDivergence, 0.5f-0.5f*m_SlimeDivergence, 0.0f);
	m_SlimeSpecies[1].sensorMask = Vector3(0.5f+0.5f*m_SlimeDivergence, 0.0f, 0.5f-0.5f*m_SlimeDivergence);
	m_SlimeSpecies[0].sensorAttraction = (1.0f/m_SlimeSpecies[0].sensorMask.Length()) * m_SlimeSpecies[0].sensorMask + m_SlimeDivergence * (1.0f/m_SlimeSpecies[1].sensorMask.Length()) * m_SlimeSpecies[1].sensorMask;
	m_SlimeSpecies[1].sensorAttraction = (1.0f/m_SlimeSpecies[1].sensorMask.Length()) * m_SlimeSpecies[1].sensorMask + m_SlimeDivergence * (1.0f/m_SlimeSpecies[0].sensorMask.Length()) * m_SlimeSpecies[0].sensorMask;

	m_SlimeAgentsShader.ClearSpeciesMaskA(context);
	m_SlimeAgentsShader.ClearSpeciesMaskB(context);
	m_SlimeAgentsShader.ClearSpeciesMaskC(context);

	//m_SlimePheremoneShader.ClearTexture(context); // NB: Turned off for 'eating' effect...

	m_SlimeAgentsShader.EnableShader(context);
	m_SlimeAgentsShader.SetSlimeBuffer(context, 0.03f - 0.025f * cos(XM_2PI * (m_timer.GetTotalSeconds() - 15.0f) / 40.0f));
	m_SlimeAgentsShader.SetTimeBuffer(context, m_timer.GetTotalSeconds(), m_timer.GetElapsedSeconds());
	m_SlimeAgentsShader.SetTextureBuffer(context);
	m_SlimeAgentsShader.SetSpeciesBuffer(context, m_SlimeSpecies);
	m_SlimeAgentsShader.SetCellBuffer(context, m_SlimeAgentsShader.GetCellBuffer(context));
	m_SlimeAgentsShader.SetPheremoneMask(context, m_SlimePheremoneShader.GetPheremoneMask(context));
	m_SlimeAgentsShader.SetSpeciesMaskA(context, m_SlimeAgentsShader.GetSpeciesMaskA(context));
	m_SlimeAgentsShader.SetSpeciesMaskB(context, m_SlimeAgentsShader.GetSpeciesMaskB(context));
	m_SlimeAgentsShader.SetSpeciesMaskC(context, m_SlimeAgentsShader.GetSpeciesMaskC(context));
	m_SlimeAgentsShader.SetFoodMask(context, m_ComputePheremones.Get());// m_SlimeAgentsShader.GetFoodMask(context));
	context->Dispatch(ceil(m_SlimeAgentsShader.m_cells/16), 1, 1);
	m_SlimeAgentsShader.DisableShader(context);

	m_SlimePheremoneShader.EnableShader(context);
	m_SlimePheremoneShader.SetSlimeBuffer(context, 0.03f - 0.025f * cos(XM_2PI * (m_timer.GetTotalSeconds() - 15.0f) / 40.0f));
	m_SlimePheremoneShader.SetTimeBuffer(context, m_timer.GetTotalSeconds(), m_timer.GetElapsedSeconds());
	m_SlimePheremoneShader.SetTextureBuffer(context);
	m_SlimePheremoneShader.SetSpeciesBuffer(context, m_SlimeSpecies);
	m_SlimePheremoneShader.SetTexture(context, m_SlimePheremoneShader.GetTexture(context));
	m_SlimePheremoneShader.SetPheremoneMask(context, m_SlimePheremoneShader.GetPheremoneMask(context));
	m_SlimePheremoneShader.SetSpeciesMaskA(context, m_SlimeAgentsShader.GetSpeciesMaskA(context));
	m_SlimePheremoneShader.SetSpeciesMaskB(context, m_SlimeAgentsShader.GetSpeciesMaskB(context));
	m_SlimePheremoneShader.SetSpeciesMaskC(context, m_SlimeAgentsShader.GetSpeciesMaskC(context));
	context->Dispatch(ceil(m_SlimePheremoneShader.m_width/16), ceil(m_SlimePheremoneShader.m_height/16), 1);
	m_SlimeAgentsShader.DisableShader(context);

	// PHYSICAL RENDER
	m_PhysicalRenderPass->setRenderTarget(context);
	m_PhysicalRenderPass->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);

	m_CubeShader.EnableShader(context);
	m_CubeShader.SetMatrixBuffer(context, &(Matrix::CreateTranslation(-0.5f, -0.5f, -0.5f)*Matrix::CreateScale(1.5f)*Matrix::CreateTranslation(-1.0f, 0.0f, -1.0f)), &m_view, &m_projection, true);
	m_CubeShader.SetTimeBuffer(context, m_timer.GetTotalSeconds());
	m_CubeShader.SetAlphaBuffer(context, 1.0f);
	m_CubeShader.SetLightBuffer(context, &m_Light);
	m_Cube.Render(context);

	m_SphereShader.EnableShader(context);
	m_SphereShader.SetMatrixBuffer(context, &(Matrix::CreateTranslation(-0.5f, -0.5f, -0.5f)*Matrix::CreateScale(1.5f)*Matrix::CreateTranslation(1.0f, 0.0f, 1.0f)), &m_view, &m_projection, true);
	m_SphereShader.SetTimeBuffer(context, m_timer.GetTotalSeconds());
	m_SphereShader.SetAlphaBuffer(context, 1.0f);
	m_SphereShader.SetLightBuffer(context, &m_Light);
	m_Sphere.Render(context);

	m_HorizontalTorusShader.EnableShader(context);
	m_HorizontalTorusShader.SetMatrixBuffer(context, &(Matrix::CreateTranslation(-0.5f, -0.5f, -0.5f)*Matrix::CreateRotationX(XM_PIDIV2)*Matrix::CreateScale(1.5f)*Matrix::CreateTranslation(1.0f, 0.0f, -1.0f)), &m_view, &m_projection, true);
	m_HorizontalTorusShader.SetTimeBuffer(context, m_timer.GetTotalSeconds());
	m_HorizontalTorusShader.SetAlphaBuffer(context, 1.0f);
	m_HorizontalTorusShader.SetLightBuffer(context, &m_Light);
	m_Torus.Render(context);

	m_VerticalTorusShader.EnableShader(context);
	m_VerticalTorusShader.SetMatrixBuffer(context, &(Matrix::CreateTranslation(-0.5f, -0.5f, -0.5f)*Matrix::CreateScale(1.5f)*Matrix::CreateTranslation(-1.0f, 0.0f, 1.0f)), &m_view, &m_projection, true);
	m_VerticalTorusShader.SetTimeBuffer(context, m_timer.GetTotalSeconds());
	m_VerticalTorusShader.SetAlphaBuffer(context, 1.0f);
	m_VerticalTorusShader.SetLightBuffer(context, &m_Light);
	m_Torus.Render(context);

	// VEINS RENDER:
	m_VeinsRenderPass->setRenderTarget(context);
	m_VeinsRenderPass->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 0.0f);

	float theta;
	for (int i = 0; i < m_BloodVesselCount; i++)
	{
		theta = i*XM_2PI/m_BloodVesselCount+XM_PI/m_BloodVesselCount;

		m_NeutralShader.EnableShader(context);
		m_NeutralShader.SetMatrixBuffer(context, &(Matrix::CreateTranslation(-0.5f, -0.5f, 0.0f)*Matrix::CreateScale(1.0f)*Matrix::CreateRotationZ(theta+XM_PIDIV2)*Matrix::CreateTranslation(0.85f*Vector3(pow(1.0f+pow(m_aspectRatio, 2.0f), 0.5f)*cos(theta), pow(1.0f/m_aspectRatio, 0.5f)*pow(1.0f+pow(m_aspectRatio, 2.0f), 0.5f)*sin(theta), 0.0f))*Matrix::CreateScale(1.0f)), &(Matrix)Matrix::Identity, &Matrix::CreateScale(1.0f/m_aspectRatio, 1.0f, 1.0f), true);
		context->RSSetState(m_states->CullCounterClockwise()); // NB: MatrixBuffer uses 'true' on both sides, since we aren't applying lighting to the model anyway!
		m_BloodVessels[i].Render(context);
		context->RSSetState(m_states->CullClockwise());
		m_BloodVessels[i].Render(context);
	}

	// COMPOSITE RENDER:
	context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);

	m_ScreenShader.EnableShader(context);
	m_ScreenShader.SetMatrixBuffer(context, &(Matrix)Matrix::Identity, &(Matrix)Matrix::Identity, &(Matrix)Matrix::Identity, true);
	m_ScreenShader.SetMatrixBuffer(context, &(Matrix::CreateScale(0.67f) * Matrix::CreateTranslation(-0.75f, 0.0f, 1.0f)), &(Matrix)Matrix::Identity, &Matrix::CreateScale(1.0f/m_aspectRatio, 1.0f, 1.0f), true);
	m_ScreenShader.SetTimeBuffer(context, m_time);
	m_ScreenShader.SetAlphaBuffer(context, 2.0f/3.0f);
	m_ScreenShader.SetAspectRatioBuffer(context, m_aspectRatio);
	m_ScreenShader.SetStressBuffer(context, 0.0f);// (m_BloodVesselCount > 0) ? *m_BloodVessels[0].GetIntensity() : 0.0f);
	m_ScreenShader.SetShaderTexture(context, m_ComputeTexture.Get()/*m_SlimePheremoneShader.GetTexture(context)*/, -1, 0);// m_PhysicalRenderPass->getShaderResourceView(), -1, 0);
	//m_ScreenShader.SetShaderTexture(context, m_SlimePheremoneShader.GetTexture(context), -1, 1);//m_VeinsRenderPass->getShaderResourceView(), -1, 1);
	m_Screen.Render(context);

	m_ScreenShader.EnableShader(context);
	m_ScreenShader.SetMatrixBuffer(context, &(Matrix)Matrix::Identity, &(Matrix)Matrix::Identity, &(Matrix)Matrix::Identity, true);
	m_ScreenShader.SetMatrixBuffer(context, &(Matrix::CreateScale(0.67f) * Matrix::CreateTranslation(-0.75f, 0.0f, 0.0f)), &(Matrix)Matrix::Identity, &Matrix::CreateScale(1.0f/m_aspectRatio, 1.0f, 1.0f), true);
	m_ScreenShader.SetTimeBuffer(context, m_time);
	m_ScreenShader.SetAlphaBuffer(context, 2.0f/3.0f);
	m_ScreenShader.SetAspectRatioBuffer(context, m_aspectRatio);
	m_ScreenShader.SetStressBuffer(context, 0.0f);// (m_BloodVesselCount > 0) ? *m_BloodVessels[0].GetIntensity() : 0.0f);
	m_ScreenShader.SetShaderTexture(context, m_SlimePheremoneShader.GetTexture(context), -1, 0);// m_PhysicalRenderPass->getShaderResourceView(), -1, 0);
	//m_ScreenShader.SetShaderTexture(context, m_SlimePheremoneShader.GetTexture(context), -1, 1);//m_VeinsRenderPass->getShaderResourceView(), -1, 1);
	m_Screen.Render(context);

	m_ScreenShader.EnableShader(context);
	m_ScreenShader.SetMatrixBuffer(context, &(Matrix)Matrix::Identity, &(Matrix)Matrix::Identity, &(Matrix)Matrix::Identity, true);
	m_ScreenShader.SetMatrixBuffer(context, &(Matrix::CreateScale(0.67f) * Matrix::CreateTranslation(0.75f, 0.0f, 0.0f)), &(Matrix)Matrix::Identity, &Matrix::CreateScale(1.0f/m_aspectRatio, 1.0f, 1.0f), true);
	m_ScreenShader.SetTimeBuffer(context, m_time);
	m_ScreenShader.SetAlphaBuffer(context, 2.0f/3.0f);
	m_ScreenShader.SetAspectRatioBuffer(context, m_aspectRatio);
	m_ScreenShader.SetStressBuffer(context, 0.0f);// (m_BloodVesselCount > 0) ? *m_BloodVessels[0].GetIntensity() : 0.0f);
	m_ScreenShader.SetShaderTexture(context, m_SlimePheremoneShader.GetPheremoneMask(context), -1, 0);// m_PhysicalRenderPass->getShaderResourceView(), -1, 0);
	//m_ScreenShader.SetShaderTexture(context, m_SlimePheremoneShader.GetTexture(context), -1, 1);//m_VeinsRenderPass->getShaderResourceView(), -1, 1);
	m_Screen.Render(context);

	//render our GUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::Black);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
	width = 1920;
	height = 1080;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	// L-Systems
	m_BloodVesselCount = 16;
	for (int i = 0; i < m_BloodVesselCount; i++)
	{
		m_BloodVessels.push_back(LBloodVessel());
		m_BloodVessels[i].Initialize(device, 0.1f, 8, std::rand());
	}

	// Models
	m_Screen.Initialize(device);

	Field cubicField = Field();
	cubicField.Initialise(16);
	cubicField.InitialiseCubicField();
	m_Cube.Initialize(device, 16, cubicField.m_field, 1.0f);

	Field sphericalField = Field();
	sphericalField.Initialise(16);
	sphericalField.InitialiseSphericalField(0);
	m_Sphere.Initialize(device, 16, sphericalField.m_field, 1.0f);

	Field toroidalField = Field();
	toroidalField.Initialise(16);
	toroidalField.InitialiseToroidalField(0.75f, 0);
	m_Torus.Initialize(device, 16, toroidalField.m_field, 1.0f);

	// Shaders
	m_NeutralShader.InitShader(device, L"neutral_vs.cso", L"neutral_ps.cso");
	m_NeutralShader.InitMatrixBuffer(device);

	m_CubeShader.InitShader(device, L"texture_3vs.cso", L"euclidean_voronoi_3ps.cso");
	m_CubeShader.InitMatrixBuffer(device);
	m_CubeShader.InitTimeBuffer(device);
	m_CubeShader.InitAlphaBuffer(device);
	m_CubeShader.InitLightBuffer(device);

	m_SphereShader.InitShader(device, L"light_evs.cso", L"pores_001_eps.cso");
	m_SphereShader.InitMatrixBuffer(device);
	m_SphereShader.InitTimeBuffer(device);
	m_SphereShader.InitAlphaBuffer(device);
	m_SphereShader.InitLightBuffer(device);

	m_HorizontalTorusShader.InitShader(device, L"texture_3vs.cso", L"texture_coordinates_3ps.cso");
	m_HorizontalTorusShader.InitMatrixBuffer(device);
	m_HorizontalTorusShader.InitTimeBuffer(device);
	m_HorizontalTorusShader.InitAlphaBuffer(device);
	m_HorizontalTorusShader.InitLightBuffer(device);

	m_VerticalTorusShader.InitShader(device, L"light_evs.cso", L"euclidean_voronoi_light_eps.cso");
	m_VerticalTorusShader.InitMatrixBuffer(device);
	m_VerticalTorusShader.InitTimeBuffer(device);
	m_VerticalTorusShader.InitAlphaBuffer(device);
	m_VerticalTorusShader.InitLightBuffer(device);

	m_ScreenShader.InitShader(device, L"vignette_vs.cso", L"vignette_ps.cso");
	m_ScreenShader.InitMatrixBuffer(device);
	m_ScreenShader.InitTimeBuffer(device);
	m_ScreenShader.InitAlphaBuffer(device);
	m_ScreenShader.InitAspectRatioBuffer(device);
	m_ScreenShader.InitStressBuffer(device);

	// Compute Shaders
	CreateDDSTextureFromFile(device, L"Picnic_001-Dithered.dds", nullptr, m_ComputeTexture.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"Picnic_001_PheremoneMask.dds", nullptr, m_ComputePheremones.ReleaseAndGetAddressOf());

	//D3D11_SHADER_RESOURCE_VIEW_DESC test;
	//m_ComputePheremones.Get()->GetDesc(&test);
	//if (test.Format == test.Format)// DXGI_FORMAT_R8G8B8A8_UINT)//DXGI_FORMAT_R32G32B32A32_FLOAT)
	//	return;

	m_SlimeAgentsShader.InitShader(device, L"slime_agent_cs.cso");
	m_SlimeAgentsShader.InitSlimeBuffer(device, 2, 16 * 1024 * 64);
	m_SlimeAgentsShader.InitTimeBuffer(device);
	m_SlimeAgentsShader.InitTextureBuffer(device, 1024, 1024);

	m_SlimeAgentsShader.InitSpeciesBuffer(device);
	m_SlimeAgentsShader.InitCellBuffer(device);
	m_SlimeAgentsShader.InitPheremoneMask(device);
	m_SlimeAgentsShader.InitSpeciesMaskA(device);
	m_SlimeAgentsShader.InitSpeciesMaskB(device);
	m_SlimeAgentsShader.InitSpeciesMaskC(device);
	m_SlimeAgentsShader.InitFoodMask(device);

	m_SlimeSpecies = new PheremoneShader::SpeciesBuffer[m_SlimeAgentsShader.m_species];

	m_SlimeSpecies[0].speed = 1.0f;
	m_SlimeSpecies[0].angularVelocity = 4.0f;
	m_SlimeSpecies[0].sensorDistance = 0.02f;
	m_SlimeSpecies[0].sensorAngle = 30.0f * (XM_PI / 180.0f);
	m_SlimeSpecies[0].sensorPixelRadius = 0;
	m_SlimeSpecies[0].pheremoneDispersion = 1.0f; // FIXME: Is this viable?

	m_SlimeSpecies[1].speed = 1.0f;
	m_SlimeSpecies[1].angularVelocity = 4.0f;
	m_SlimeSpecies[1].sensorDistance = 0.02f;
	m_SlimeSpecies[1].sensorAngle = 30.0f * (XM_PI / 180.0f);
	m_SlimeSpecies[1].sensorPixelRadius = 0;
	m_SlimeSpecies[1].pheremoneDispersion = 1.0f; // FIXME: Is this viable?

	m_SlimeSpecies[0].colour = Vector4(0.75f + 0.25f * sin(XM_2PI * m_timer.GetTotalSeconds()), 0.75f - 0.25f * sin(XM_2PI * m_timer.GetTotalSeconds()), 1.0f, 1.0f);
	m_SlimeSpecies[1].colour = Vector4(0.75f - 0.25f * sin(XM_2PI * m_timer.GetTotalSeconds()), 0.75f + 0.25f * sin(XM_2PI * m_timer.GetTotalSeconds()), 1.0f, 1.0f);

	m_SlimeDivergence = 1.0f;
	m_SlimeSpecies[0].sensorMask = Vector3(0.5f+0.5f*m_SlimeDivergence, 0.5f-0.5f*m_SlimeDivergence, 0.0f);
	m_SlimeSpecies[1].sensorMask = Vector3(0.5f+0.5f*m_SlimeDivergence, 0.0f, 0.5f-0.5f*m_SlimeDivergence);
	m_SlimeSpecies[0].sensorAttraction = (1.0f/m_SlimeSpecies[0].sensorMask.Length()) * m_SlimeSpecies[0].sensorMask + m_SlimeDivergence * (1.0f/m_SlimeSpecies[1].sensorMask.Length()) * m_SlimeSpecies[1].sensorMask;
	m_SlimeSpecies[1].sensorAttraction = (1.0f/m_SlimeSpecies[1].sensorMask.Length()) * m_SlimeSpecies[1].sensorMask + m_SlimeDivergence * (1.0f/m_SlimeSpecies[0].sensorMask.Length()) * m_SlimeSpecies[0].sensorMask;

	m_SlimeSpecies[0].sensorMask = Vector3(1.0f, 0.0f, 0.0f);
	m_SlimeSpecies[1].sensorMask = Vector3(0.0f, 1.0f, 0.0f);
	m_SlimeSpecies[0].sensorAttraction = Vector3(1.0f, m_SlimeDivergence, -1.0f);
	m_SlimeSpecies[1].sensorAttraction = Vector3(m_SlimeDivergence, 1.0f, -1.0f);

	PheremoneShader::CellBuffer* cells = new PheremoneShader::CellBuffer[m_SlimeAgentsShader.m_cells];
	for (int i = 0; i < m_SlimeAgentsShader.m_cells; i++)
	{
		cells[i].species = i% m_SlimeAgentsShader.m_species;
		cells[i].position = Vector2(0.5f, 0.5f)+1.5f*Vector2(cos(i*XM_2PI/m_SlimeAgentsShader.m_cells), sin(i*XM_2PI/m_SlimeAgentsShader.m_cells));
		cells[i].angle = i*XM_2PI/m_SlimeAgentsShader.m_cells;
	}

	m_SlimeAgentsShader.EnableShader(context);
	m_SlimeAgentsShader.SetSlimeBuffer(context, 0.0f);
	m_SlimeAgentsShader.SetTimeBuffer(context, 0.0f, 0.0f);
	m_SlimeAgentsShader.SetTextureBuffer(context);
	m_SlimeAgentsShader.SetSpeciesBuffer(context, m_SlimeSpecies);
	m_SlimeAgentsShader.SetCellBuffer(context, cells);
	m_SlimeAgentsShader.SetPheremoneMask(context, m_SlimeAgentsShader.GetPheremoneMask(context));
	m_SlimeAgentsShader.SetSpeciesMaskA(context, m_SlimeAgentsShader.GetSpeciesMaskA(context));
	m_SlimeAgentsShader.SetSpeciesMaskB(context, m_SlimeAgentsShader.GetSpeciesMaskB(context));
	m_SlimeAgentsShader.SetSpeciesMaskC(context, m_SlimeAgentsShader.GetSpeciesMaskC(context));
	m_SlimeAgentsShader.SetFoodMask(context, m_ComputePheremones.Get());
	context->Dispatch(ceil(m_SlimeAgentsShader.m_cells/16), 1, 1);
	m_SlimeAgentsShader.DisableShader(context);

	m_SlimePheremoneShader.InitShader(device, L"slime_pheremone_cs.cso");
	m_SlimePheremoneShader.InitSlimeBuffer(device, m_SlimeAgentsShader.m_species, m_SlimeAgentsShader.m_cells);
	m_SlimePheremoneShader.InitTimeBuffer(device);
	m_SlimePheremoneShader.InitTextureBuffer(device, m_SlimeAgentsShader.m_width, m_SlimeAgentsShader.m_height);

	m_SlimePheremoneShader.InitSpeciesBuffer(device);
	m_SlimePheremoneShader.InitTexture(device);
	m_SlimePheremoneShader.InitPheremoneMask(device);
	m_SlimePheremoneShader.InitSpeciesMaskA(device);
	m_SlimePheremoneShader.InitSpeciesMaskB(device);
	m_SlimePheremoneShader.InitSpeciesMaskC(device);

	m_SlimePheremoneShader.EnableShader(context);
	m_SlimePheremoneShader.SetSlimeBuffer(context, 0.0f);
	m_SlimePheremoneShader.SetTimeBuffer(context, 0.0f, 0.0f);
	m_SlimePheremoneShader.SetTextureBuffer(context);
	m_SlimePheremoneShader.SetSpeciesBuffer(context, m_SlimeSpecies);
	m_SlimePheremoneShader.SetTexture(context, m_SlimePheremoneShader.GetTexture(context));
	m_SlimePheremoneShader.SetPheremoneMask(context, m_SlimePheremoneShader.GetPheremoneMask(context));
	m_SlimePheremoneShader.SetSpeciesMaskA(context, m_SlimeAgentsShader.GetSpeciesMaskA(context));
	m_SlimePheremoneShader.SetSpeciesMaskB(context, m_SlimeAgentsShader.GetSpeciesMaskB(context));
	m_SlimePheremoneShader.SetSpeciesMaskC(context, m_SlimeAgentsShader.GetSpeciesMaskC(context));
	context->Dispatch(ceil(m_SlimePheremoneShader.m_width/16), ceil(m_SlimePheremoneShader.m_height/16), 1);
	m_SlimeAgentsShader.DisableShader(context);

	//load Textures
	CreateDDSTextureFromFile(device, L"LocationMarker_002.dds", nullptr, m_normalMap.ReleaseAndGetAddressOf());

	//Initialise Render to texture
	m_PhysicalRenderPass = new RenderTexture(device, 1920, 1080, 1, 2); // FIXME: How do I make this 2048x2048?
	m_VeinsRenderPass = new RenderTexture(device, 1920, 1080, 1, 2); // FIXME: How do I make this 2048x2048?

	m_preRendered = false;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();

	m_width = size.right;
	m_height = size.bottom;
	m_aspectRatio = (float)m_width/m_height;
	m_fov = 50.0f * XM_PI / 180.0f;

    // This sample makes use of a right-handed coordinate system using row-major matrices.
	m_Camera.setPerspective(m_fov, m_aspectRatio, 0.01f, 100.0f);

	//Initialise Render to texture?
	//auto device = m_deviceResources->GetD3DDevice();
	//m_PhysicalRenderPass = new RenderTexture(device, m_width, m_height, 1, 2); // FIXME: How do I make this 2048x2048?
	//m_VeinsRenderPass = new RenderTexture(device, m_width, m_height, 1, 2); // FIXME: How do I make this 2048x2048?
}

int Game::SetupGUI(std::string text, std::vector<std::string> choices)
{
	int choice = -1;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoCollapse;

	SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.825f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	float textWidth = std::max(720.0f, std::min(ImGui::CalcTextSize(text.c_str()).x, 1440.0f));

	ImGui::Begin((text.length() > 0) ? text.c_str() : " ", (bool*)true, window_flags); // NB: Nice, robust failsafe here?

	// FIXME: How do we right align wrapped text? (without a ridiculous amount of code...)
	ImGui::PushTextWrapPos(1440.0f);
	ImGui::Text(text.c_str(), ImVec2(textWidth, 0.0f));
	ImGui::PopTextWrapPos();

	// FIXME: StoryEngine buttons will be handled here...
	ImGui::PushFont(m_choiceFont);
	for (int i = 0; i < choices.size(); i++)
	{
		// FIXME: Needed to right align buttons...
		/*ImGui::Dummy(ImVec2(0.25f*textWidth, 0.0f));
		ImGui::SameLine();*/

		ImGui::PushID(i);
		if (ImGui::Button(("  "+std::to_string(i + 1)+". "+choices[i]).c_str(), ImVec2(0.9f*textWidth, 1.1f*m_choiceFont->FontSize)) || m_gameInputCommands.chooseReleased[i]) // NB: Input class has 'on released' logic in place, to stop the player skipping through storylets...
		{
			choice = i;
		}
		ImGui::PopID();
	}
	ImGui::PopFont();

	ImGui::End();

	ImGui::EndFrame();

	return choice;
}

void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
	m_testmodel.reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
