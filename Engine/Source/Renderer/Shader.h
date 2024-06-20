#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <unordered_map>
#include <glm/ext/vector_float3.hpp>

#include "Math/MathTypes.h"

enum EShaderType : uint8
{
	RK_SHADERTYPE_NONE = 0,
	RK_SHADERTYPE_VERTEXSHADER,
	RK_SHADERTYPE_FRAGMENTSHADER
};

struct SShaderProgram
{
	VkPipelineShaderStageCreateInfo CreateInfo;

	// Compiled SPIR-V bytecode. Note: Should be freed right after compilation as Vulkan creates an internal copy.
	VkShaderModule ShaderModule;
};

class RkShader
{
public:
	void Compile(EShaderType ShaderType, const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile);
	void PostCompile();

	std::vector<SShaderProgram> ShaderPrograms;
};