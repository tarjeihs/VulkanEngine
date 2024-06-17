#pragma once

#include <chrono>
#include <unordered_map>
#include <utility>

#include "KeyCode.h"
#include "Math/MathTypes.h"

struct SKeyPressData
{
    std::chrono::steady_clock::time_point Start;

    bool bIsPressed;
};

class CInput
{
public:
    inline static bool KeyPress(int keyCode)
    {
        return GInput->KeyPressedImpl(keyCode);
    }

    inline static bool KeyHold(int keyCode, float Duration)
    {
        return GInput->KeyHoldImpl(keyCode, Duration);
    }

    inline static bool MouseButtonPressed(int keyCode)
    {
        return GInput->MouseButtonPressedImpl(keyCode);
    }

    inline static float GetMouseX()
    {
        return GInput->GetMouseXImpl();
    }

    inline static float GetMouseY()
    {
        return GInput->GetMouseYImpl();
    }

    inline static std::pair<float, float> GetMousePosition()
    {
        return GInput->GetMousePositionImpl();
    }

    inline static SKeyPressData& GetKeyPressData(int32 KeyCode)
    {
        return GInput->KeyPressData[KeyCode];
    }
    
protected:
    virtual bool KeyPressedImpl(int keyCode) = 0;
    virtual bool KeyHoldImpl(int keyCode, float Duration) = 0;
    virtual bool MouseButtonPressedImpl(int keyCode) = 0;

    virtual float GetMouseXImpl() = 0;
    virtual float GetMouseYImpl() = 0;
    
    virtual std::pair<float, float> GetMousePositionImpl() = 0;

protected:
    std::unordered_map<int32, SKeyPressData> KeyPressData;
    
private:
    static CInput* GInput;

};
