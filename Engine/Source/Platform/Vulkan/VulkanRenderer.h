#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/RendererPipeline.h"

class CVulkanRenderer : public CRenderer
{
public:
    virtual void Init() override;
    virtual void Cleanup() override;
    virtual void BeginFrame() override {}
    virtual void EndFrame() override {}
};
