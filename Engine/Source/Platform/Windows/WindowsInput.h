#pragma once
#include "Core/Input.h"

class CWindowsInput : public CInput
{
protected:
    virtual bool KeyPressedImpl(int keyCode) override;
    virtual bool KeyHoldImpl(int keyCode, float Threshold) override;
    virtual bool MouseButtonPressedImpl(int keyCode) override;

    virtual float GetMouseXImpl() override;
    virtual float GetMouseYImpl() override;

    virtual std::pair<float, float> GetMousePositionImpl() override;
};
