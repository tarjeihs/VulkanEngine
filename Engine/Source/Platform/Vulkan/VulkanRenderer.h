#pragma once

#include "Renderer/Renderer.h"

class CVulkanRenderer : public CRenderer
{
public:
    virtual void BeginFrame() override {}
    virtual void EndFrame() override {}
};
