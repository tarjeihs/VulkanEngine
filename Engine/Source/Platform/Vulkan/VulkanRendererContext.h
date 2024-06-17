#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>

#include "Renderer/RendererContext.h"

class RkVulkanValidationLayer
{
public:
    RkVulkanValidationLayer(const char* InValidationLayerName)
        : ValidationLayerName(InValidationLayerName)
    {
    }

    bool IsSupported();
    void Enable(VkInstanceCreateInfo& CreateInfo);
private:
    const char* ValidationLayerName;

    bool bIsValidFlag = true;

    
};

class RkVulkanDebugMessenger
{
public:
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT Severity, 
        VkDebugUtilsMessageTypeFlagsEXT Type, 
        const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, 
        void* UserData);

    void SetupDebugMessenger(VkInstance Instance);
    void DestroyDebugMessenger(VkInstance Instance);
    VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo();
    VkDebugUtilsMessengerEXT GetDebugMessenger() const;
private:
    VkDebugUtilsMessengerEXT DebugMessenger;
};

class CVulkanRendererContext : public CRendererContext
{    
public:
    virtual void Init() override;
    virtual void Destroy() override;

private:
    //std::vector<RkVulkanValidationLayer> ValidationLayers;
    //std::vector<RkVulkanDebugMessenger> DebugMessengers;
    std::shared_ptr<RkVulkanDebugMessenger> DebugMessenger;
};
