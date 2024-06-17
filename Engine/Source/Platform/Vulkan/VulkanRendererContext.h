#pragma once

#include <vulkan/vulkan_core.h>

#include "Renderer/RendererContext.h"

class CVulkanRendererContext : public CRendererContext
{    
public:
    virtual void Init() override;
    virtual void Destroy() override;
};
