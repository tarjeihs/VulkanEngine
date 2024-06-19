#include "EnginePCH.h"
#include "Engine.h"

#include <iostream>
#include <sstream>

#include "Scene.h"
#include "Window.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Log.h"


CEngine* CEngine::GEngine = nullptr;

void CEngine::Start()
{
    GEngine = this;
    
    CLog::Init();

    Window = new CWindowsWindow(SWindowSpecification { "Rocket Engine", PARAMETER_VIEWPORT_WIDTH, PARAMETER_VIEWPORT_HEIGHT } );
    Window->CreateNativeWindow();
    Renderer = new CVulkanRenderer();
    Renderer->Init();
    Scene = new CScene();

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
    
    Renderer->Cleanup();
    Window->DestroyWindow();
    
    delete Window;
    delete Renderer;
    delete Scene;

    GEngine = nullptr;
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