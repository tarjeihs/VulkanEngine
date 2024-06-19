#include "Shader.h"

#include <Windows.h>
#include <wrl.h>
#include <dxcapi.h>
#include <vector>

#include "Core/Assert.h"
#include "Platform/Vulkan/VulkanRendererContext.h"
#include "Core/Window.h"
#include "Memory/Mem.h"

using Microsoft::WRL::ComPtr;

void RkShader::Compile(EShaderType ShaderType, const std::wstring& ShaderSourcePath, const std::wstring& Entrypoint, const std::string& TargetProfile)
{
    ComPtr<IDxcCompiler> Compiler;
    ComPtr<IDxcLibrary> Library;
    ComPtr<IDxcBlobEncoding> SourceBlob;
    ComPtr<IDxcOperationResult> OperationResult;

    // Create the DXC library instance
    HRESULT DxcResult = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));
    RK_ENGINE_ASSERT(!FAILED(DxcResult), "Failed to create DXC Library instance.");

    // Create the DXC compiler instance
    DxcResult = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
    RK_ENGINE_ASSERT(!FAILED(DxcResult), "Failed to create DXC Compiler instance.");

    // Load and encode the shader source file
    DxcResult = Library->CreateBlobFromFile(ShaderSourcePath.c_str(), nullptr, &SourceBlob);
    RK_ENGINE_ASSERT(!FAILED(DxcResult), "Failed to load shader source file.");

    // Compilation arguments
    std::wstring TargetProfileW(TargetProfile.begin(), TargetProfile.end());
    std::vector<LPCWSTR> Arguments;
    Arguments.push_back(L"-E");
    Arguments.push_back(Entrypoint.c_str());
    Arguments.push_back(L"-T");
    Arguments.push_back(TargetProfileW.c_str());
    Arguments.push_back(L"-spirv");
    Arguments.push_back(L"-fvk-use-dx-layout");

    // Compile HLSL into SPIR-V bytecode using DirectX Shader Compiler
    DxcResult = Compiler->Compile(
        SourceBlob.Get(), 
        ShaderSourcePath.c_str(), 
        Entrypoint.c_str(), 
        TargetProfileW.c_str(), 
        Arguments.data(), 
        Arguments.size(), 
        nullptr,
        0, 
        nullptr, 
        &OperationResult);
    RK_ENGINE_ASSERT(!FAILED(DxcResult), "Shader compilation failed.");

    HRESULT CompileStatus;
    DxcResult = OperationResult->GetStatus(&CompileStatus);
    RK_ENGINE_ASSERT(!FAILED(DxcResult), "Undefined compile status.");

    if (FAILED(CompileStatus))
    {
        ComPtr<IDxcBlobEncoding> ErrorBlob;
        DxcResult = OperationResult->GetErrorBuffer(&ErrorBlob);
        if (SUCCEEDED(DxcResult) && ErrorBlob)
        {
            std::string errorMsg((char*)ErrorBlob->GetBufferPointer(), ErrorBlob->GetBufferSize());
            RK_ENGINE_ERROR("Shader Compilation Error: %s", errorMsg);
        }
    }

    // Containing the compiled shader
    ComPtr<IDxcBlob> ShaderBlob;
    DxcResult = OperationResult->GetResult(&ShaderBlob);
    RK_ENGINE_ASSERT(!FAILED(DxcResult), "Unable to retrieve compiled shader.");

    // Shader module configuration
    VkShaderModuleCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.codeSize = ShaderBlob->GetBufferSize();
    CreateInfo.pCode = reinterpret_cast<const uint32*>(ShaderBlob->GetBufferPointer());

    RkVulkanRendererContext* RendererContext = Cast<RkVulkanRendererContext>(GetWindow()->GetContext());
    
    VkShaderModule ShaderModule;
    VkResult Result = vkCreateShaderModule(RendererContext->GetLogicalDevice(), &CreateInfo, nullptr, &ShaderModule);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Shader Module.");

    // TODO: Currently not doing anything with this CreateInfo.
    VkPipelineShaderStageCreateInfo ShaderCreateInfo{};
    ShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ShaderCreateInfo.pName = "main";

    switch (ShaderType)
    {
        case EShaderType::VertexShader: { ShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break; }
        case EShaderType::FragmentShader: { ShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break; }
    }
    
    ShaderModules[ShaderType] = ShaderModule;
}

void RkShader::Cleanup()
{
    RkVulkanRendererContext* RendererContext = Cast<RkVulkanRendererContext>(GetWindow()->GetContext());
    for (const auto& Pair : ShaderModules)
    {
        vkDestroyShaderModule(RendererContext->GetLogicalDevice(), Pair.second, nullptr);
    }
}
