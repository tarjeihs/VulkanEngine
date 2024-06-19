#include "EnginePCH.h"
#include "Engine.h"

#include <iostream>
#include <sstream>

#include "Scene.h"
#include "Window.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Log.h"
#include "Renderer/Shader.h"

CEngine* CEngine::GEngine = nullptr;

void CEngine::Start()
{
    GEngine = this;
    
    CLog::Init();

    Window = new CWindowsWindow(SWindowSpecification { "Rocket Engine", PARAMETER_VIEWPORT_WIDTH, PARAMETER_VIEWPORT_HEIGHT } );
    Window->CreateNativeWindow();
    Renderer = new CVulkanRenderer();
    Scene = new CScene();

    RkShader Shader;
    std::wstring VertexPath = L"../Shaders/SimpleShaderVert.hlsl";
    std::wstring VertexEntrypoint = L"main";
    std::string VertexTargetProfile = "vs_6_0"; // SM6
    std::wstring FragmentPath = L"../Shaders/SimpleShaderFrag.hlsl";
    std::wstring FragmentEntrypoint = L"main";
    std::string FragmentTargetProfile = "ps_6_0"; // SM6
    Shader.Compile(EShaderType::VertexShader, VertexPath, VertexEntrypoint, VertexTargetProfile);
    Shader.Compile(EShaderType::FragmentShader, FragmentPath, FragmentEntrypoint, FragmentTargetProfile);
    Shader.Cleanup();
    
    OnStart();
}

void CEngine::Run()
{
    while (!Window->ShouldClose())
    {
        Metrics.Reset();
        Time.Validate();

        Window->Poll();
        OnUpdate(Time.GetDeltaTime());
        Scene->Tick(Time.GetDeltaTime());

        Renderer->BeginFrame();
        //for (CActor* Actor : Scene->GetActors())
        //{
        //    Actor->Tick(Time.GetDeltaTime());
        //}
        Renderer->EndFrame();

        Window->Swap();
    }
}

void CEngine::Stop()
{
    OnStop();
    
    Window->DestroyWindow();
    
    delete Window;
}

CWindow* CEngine::GetWindow() const
{
    return Window;
}

CScene* CEngine::GetScene() const
{
    return Scene;
}

CRenderer* CEngine::GetRenderer() const
{
    return Renderer;
}