#include "EnginePCH.h"
#include "VulkanRendererPipeline.h"

#include <vector>

#include "Core/Window.h"
#include "Math/MathTypes.h"
#include "Memory/Mem.h"
#include "VulkanRendererContext.h"

void CVulkanRenderPipeline::Init()
{
	// Configure the pipeline states explicitly as it will be baked into an immutable pipeline state object. 

	// Format of vertex data passed into vertex shader
	VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo{};
	VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	VertexInputCreateInfo.vertexAttributeDescriptionCount = 0;

	// NOTE: Without an index buffer(IBO), we cannot perform optimizations like reusing vertices.
	// Geometry topology and primitive restart
	VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo{};
	InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	InputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Triangle from every 3 vertices without reuse
	InputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	// Describes the region of the framebuffer that the output will be rendered to.
	VkPipelineViewportStateCreateInfo ViewportCreateInfo{};
	ViewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ViewportCreateInfo.viewportCount = 1;
	ViewportCreateInfo.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo{};
	RasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizerCreateInfo.depthClampEnable = VK_FALSE;
	RasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	RasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // Determines how fragments are generated on the geometry. VK_POLYGON_MODE_FILL will fill the area of the polygon with fragments.
	RasterizerCreateInfo.lineWidth = 1.0f; // Thickness of lines in terms of number of fragments.
	RasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	RasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // Vertex order for faces to be considered front-facing.
	RasterizerCreateInfo.depthBiasEnable = VK_FALSE;

	// Anti-aliasing
	VkPipelineMultisampleStateCreateInfo MultisampleCreateInfo{};
	MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	MultisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	MultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer.
	// Blending the new color with the old color based on its opacity. (Alpha blending)
	VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
	ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	ColorBlendAttachment.blendEnable = VK_TRUE;
	ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo ColorBlendCreateInfo{};
	ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendCreateInfo.logicOpEnable = VK_FALSE;
	ColorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	ColorBlendCreateInfo.attachmentCount = 1;
	ColorBlendCreateInfo.pAttachments = &ColorBlendAttachment;
	ColorBlendCreateInfo.blendConstants[0] = 0.0f;
	ColorBlendCreateInfo.blendConstants[1] = 0.0f;
	ColorBlendCreateInfo.blendConstants[2] = 0.0f;
	ColorBlendCreateInfo.blendConstants[3] = 0.0f;

	const std::vector<VkDynamicState> DynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo{};
	DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32>(DynamicStates.size());
	DynamicStateCreateInfo.pDynamicStates = DynamicStates.data();

	VkPipelineLayoutCreateInfo PipelineCreateInfo{};
	PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineCreateInfo.setLayoutCount = 0;
	PipelineCreateInfo.pushConstantRangeCount = 0;

	RkVulkanRendererContext* RendererContext = Cast<RkVulkanRendererContext>(GetWindow()->GetContext());

	VkResult Result = vkCreatePipelineLayout(RendererContext->GetLogicalDevice(), &PipelineCreateInfo, nullptr, &Pipeline);
	RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan pipeline layout.");
}

void CVulkanRenderPipeline::Teardown()
{
	RkVulkanRendererContext* RendererContext = Cast<RkVulkanRendererContext>(GetWindow()->GetContext());

	vkDestroyPipelineLayout(RendererContext->GetLogicalDevice(), Pipeline, nullptr);
}
