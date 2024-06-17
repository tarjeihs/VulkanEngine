#include "EnginePCH.h"
#include "WindowsWindow.h"

#include <iostream>
#include <glfw/glfw3.h>

#include "Platform/Vulkan/VulkanRendererContext.h"

void CWindowsWindow::CreateNativeWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    WindowHandle = glfwCreateWindow(Specification.Width, Specification.Height, Specification.Title, nullptr, nullptr);
    if (WindowHandle == nullptr)
    {
        glfwTerminate();
        ASSERT(false, "Failed to create GLFW window");
    }

    glfwSetWindowUserPointer((GLFWwindow*)WindowHandle, &UserData);
    glfwSetInputMode((GLFWwindow*)WindowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwMakeContextCurrent((GLFWwindow*)WindowHandle);
    glfwSwapInterval(1); // Enable vsync
    
    // Vulkan Context
    RendererContext = new CVulkanRendererContext();
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
}

void CWindowsWindow::Swap()
{
    glfwSwapBuffers((GLFWwindow*)WindowHandle);
}

bool CWindowsWindow::ShouldClose() const
{
    return glfwWindowShouldClose((GLFWwindow*)WindowHandle);
}