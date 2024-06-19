#include "EnginePCH.h"
#include "VulkanRenderer.h"

#include "Math/Transform.h"
#include "Platform/Vulkan/VulkanRendererPipeline.h"

void CVulkanRenderer::Init()
{
	RenderPipeline = new CVulkanRenderPipeline();
	RenderPipeline->Init();
}

void CVulkanRenderer::Cleanup()
{
	RenderPipeline->Teardown();
	delete RenderPipeline;
}
