#include "EnginePCH.h"
#include "WindowsInput.h"

#include <glfw/glfw3.h>

#include "Core/Engine.h"
#include "Core/Window.h"

CInput* CInput::GInput = new CWindowsInput();

bool CWindowsInput::KeyPressedImpl(int keyCode)
{
    auto Window = static_cast<GLFWwindow*>(GetEngine()->GetWindow()->GetNativeWindow());
    auto State = glfwGetKey(Window, keyCode);

    return State == GLFW_PRESS || State == GLFW_REPEAT;
}

bool CWindowsInput::KeyHoldImpl(int KeyCode, float Threshold)
{
    auto Now = std::chrono::steady_clock::now();

    if (KeyPressData[KeyCode].bIsPressed)
    {
        auto Duration = std::chrono::duration<double>(Now - KeyPressData[KeyCode].Start).count();
        if (Duration >= Threshold)
        {
            KeyPressData[KeyCode].bIsPressed = false;
            return true;
        }
    }
    return false;
}

bool CWindowsInput::MouseButtonPressedImpl(int keyCode)
{
    auto window = static_cast<GLFWwindow*>(GetEngine()->GetWindow()->GetNativeWindow());
    auto state = glfwGetMouseButton(window, keyCode);

    return state == GLFW_PRESS;
}

float CWindowsInput::GetMouseXImpl()
{
    auto[MouseX, MouseY] = GetMousePositionImpl();
    return MouseX;
}

float CWindowsInput::GetMouseYImpl()
{
    auto[MouseX, MouseY] = GetMousePositionImpl();
    return MouseY;
}

std::pair<float, float> CWindowsInput::GetMousePositionImpl()
{
    auto Window = static_cast<GLFWwindow*>(GetEngine()->GetWindow()->GetNativeWindow());
    double MouseX, MouseY;
    glfwGetCursorPos(Window, &MouseX, &MouseY);
    return { (float)MouseX, (float)MouseY };
}
