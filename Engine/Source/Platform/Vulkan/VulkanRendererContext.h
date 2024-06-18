#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <optional>

#include "Renderer/RendererContext.h"
#include "Math/MathTypes.h"
#include "Core/Assert.h"

class RkVulkanValidationLayer
{
public:
    RkVulkanValidationLayer(const char* InValidationLayerName)
        : Name(InValidationLayerName)
    {
        RK_ENGINE_ASSERT(IsSupported(), "This Validation Layer is not supported.");
    }

    bool IsSupported();
    
    const char* Name;
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

struct RkQueueFamilyIndices
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    inline bool IsComplete() const
    {
        return GraphicsFamily.has_value() && PresentFamily.has_value();
    }
};

class RkVulkanRendererContext : public CRendererContext
{    
public:
    virtual void Init() override;
    virtual void Destroy() override;

protected:
    void CreateVulkanInstance();
    void CreateWindowSurface();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();

private:
    VkQueue GraphicsQueue;
    VkSurfaceKHR Surface;
    VkDevice LogicalDevice;
    VkPhysicalDevice PhysicalDevice;
    RkQueueFamilyIndices QueueFamily;

    std::shared_ptr<RkVulkanDebugMessenger> DebugMessenger;
    std::vector<RkVulkanValidationLayer> ValidationLayers;
};
