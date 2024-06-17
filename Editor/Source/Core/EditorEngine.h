#pragma once

#include "Core/Window.h"

class CEditorEngine : public CEngine
{
public:
    virtual void OnStart() override;
    virtual void OnUpdate(float DeltaTime) override;

private:
    bool bDebugDraw = false;
    bool bCursorMode = false;
};