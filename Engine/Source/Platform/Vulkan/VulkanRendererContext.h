#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <optional>

#include "Renderer/RendererContext.h"
#include "Math/MathTypes.h"
#include "Core/Assert.h"

class RkValidationLayer
{
public:
    RkValidationLayer(const char* InValidationLayerName)
        : Name(InValidationLayerName)
    {
        RK_ENGINE_ASSERT(IsSupported(), "This Validation Layer is not supported.");
    }

    bool IsSupported();
    
    const char* Name;
    bool bIsValidFlag = true;
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

struct RkSwapChainSupportDetails
{
    // Swapchain capabilities (eg. min/max images in swap chain, min/max resolution of images)
    VkSurfaceCapabilitiesKHR Capabilities;

    // Pixel format, color depth
    std::vector<VkSurfaceFormatKHR> Formats;

    // Conditions for "swapping" images to the screen
    std::vector<VkPresentModeKHR> PresentMode;
};

class RkVulkanRendererContext : public CRendererContext
{    
public:
    virtual void Init() override;
    virtual void Destroy() override;

protected:
    void CreateInstance();
    void CreateSurfaceInterface();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();

    void SetupDebugMessenger(VkInstance Instance);
    void DestroyDebugMessenger(VkInstance Instance);
    VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo();
    VkDebugUtilsMessengerEXT GetDebugMessenger() const;

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
            VkDebugUtilsMessageTypeFlagsEXT Type,
            const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
            void* UserData);

    VkSurfaceFormatKHR SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats);
    VkPresentModeKHR SelectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& PresentModes);
    VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities);
    
    RkSwapChainSupportDetails RequestSwapchainSupportDetails(VkPhysicalDevice PhysicalDevice);
    RkQueueFamilyIndices RequestQueueFamilies(VkPhysicalDevice PhysicalDevice);
    bool IsVulkanCapableDevice(VkPhysicalDevice PhysicalDevice);

private:
    VkQueue GraphicsQueue;
    VkSurfaceKHR SurfaceInterface;
    VkDevice LogicalDevice;
    VkPhysicalDevice PhysicalDevice;
    VkDebugUtilsMessengerEXT DebugMessenger;
    VkSwapchainKHR Swapchain;
    VkFormat SwapchainImageFormat;
    VkExtent2D SwapchainExtent;

    std::vector<VkImage> SwapchainImages;
    std::vector<RkValidationLayer> ValidationLayers;
    std::vector<const char*> Extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};
