#pragma once

#include "Renderer/RendererPipeline.h"

#include <vulkan/vulkan.h>

class CVulkanRenderPipeline : public CRenderPipeline
{
public:
	virtual void Init() override;
	virtual void Teardown() override;

private:
	VkPipelineLayout Pipeline;
};