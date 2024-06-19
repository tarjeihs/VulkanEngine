#include "EnginePCH.h"
#include "WindowsWindow.h"

#include <iostream>
#include <glfw/glfw3.h>

#include "Platform/Vulkan/VulkanRendererContext.h"
#include "Memory/Mem.h"

void CWindowsWindow::CreateNativeWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    WindowHandle = glfwCreateWindow(Specification.Width, Specification.Height, Specification.Title, nullptr, nullptr);
    RK_ENGINE_ASSERT(WindowHandle, "Failed to create GLFW window");

    glfwSetWindowUserPointer((GLFWwindow*)WindowHandle, &UserData);
    glfwSetInputMode((GLFWwindow*)WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwMakeContextCurrent((GLFWwindow*)WindowHandle);
    glfwSwapInterval(1); // Enable vsync
    
    glfwSetFramebufferSizeCallback((GLFWwindow*)WindowHandle, [](GLFWwindow* GlfwWindow, int Width, int Height)
    {
        CWindow* Window = GetWindow();
        
        // Handle edgecase where framebuffer size is 0 (minimized)
        if (Width == 0 || Height == 0)
        {
            if (!Window->IsMinimized()) Window->SetIsMinimized(true);
        }
        else
        {
            if (Window->IsMinimized()) Window->SetIsMinimized(false);
        }
            
        RkVulkanRendererContext* Context = Cast<RkVulkanRendererContext>(GetWindow()->GetContext());
        Context->RegenerateSwapchain();
    });

    // Vulkan Context
    RendererContext = new RkVulkanRendererContext();
    RendererContext->Init();
}

void CWindowsWindow::DestroyWindow()
{
    RendererContext->Destroy();
    delete RendererContext;
    
    glfwDestroyWindow((GLFWwindow*)WindowHandle);
    glfwTerminate();
}

void CWindowsWindow::Poll()
{
    glfwPollEvents();

    int32 Width = 0, Height = 0;
    do
    {
        glfwGetFramebufferSize((GLFWwindow*)GetWindow()->GetNativeWindow(), &Width, &Height);
        glfwWaitEvents();
    } while (Width <= 0 || Height <= 0);
}

void CWindowsWindow::Swap()
{
    glfwSwapBuffers((GLFWwindow*)WindowHandle);
}

bool CWindowsWindow::ShouldClose() const
{
    return glfwWindowShouldClose((GLFWwindow*)WindowHandle);
}