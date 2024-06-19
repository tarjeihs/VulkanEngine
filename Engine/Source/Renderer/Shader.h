#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <unordered_map>

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
	VkShaderModule ShaderModule;
};

class RkShader
{
public:
	void Compile(EShaderType ShaderType, const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile);
	void PostCompile();

	std::vector<SShaderProgram> ShaderPrograms;
};