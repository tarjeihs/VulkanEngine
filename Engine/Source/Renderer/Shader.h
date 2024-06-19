#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <unordered_map>

#include "Math/MathTypes.h"

enum class EShaderType : uint8
{
	None = 0,
	
	VertexShader,

	FragmentShader
};

class RkShader
{
public:
	void Compile(EShaderType ShaderType, const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile);
	void Cleanup();

private:
	std::unordered_map<EShaderType, VkShaderModule> ShaderModules;
};