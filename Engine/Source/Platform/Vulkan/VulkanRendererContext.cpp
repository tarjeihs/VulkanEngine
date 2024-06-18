#include "VulkanRendererContext.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32

#include <cassert>
#include <stdexcept>
#include <set>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_core.h>

#include "Core/Assert.h"
#include "Core/Window.h"
#include "Math/Math.h"

namespace Utils
{
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
}

/* RkVulkanDebugMessenger */

VKAPI_ATTR VkBool32 VKAPI_CALL RkVulkanRendererContext::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
    switch (Severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: { RK_ENGINE_VERBOSE(CallbackData->pMessage);    break; }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: { RK_ENGINE_INFO(CallbackData->pMessage);       break; }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: { RK_ENGINE_WARNING(CallbackData->pMessage);    break; }
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: { RK_ENGINE_ERROR(CallbackData->pMessage);      break; }
    }
    return VK_FALSE;
}

VkSurfaceFormatKHR RkVulkanRendererContext::SelectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& Formats)
{
    for (const auto& Format : Formats)
    {
        // Prefer SRGB if available (results in more accurate perceived colors and is the golden standard).
        if (Format.format == VK_FORMAT_B8G8R8A8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return Format;
        }
    }
    return Formats[0];
}

VkPresentModeKHR RkVulkanRendererContext::SelectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& PresentModes)
{
    for (const auto& PresentMode : PresentModes)
    {
        // Instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones. 
        // This mode can be used to render frames as fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync
        if (PresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return PresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RkVulkanRendererContext::SelectSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
{
    if (Capabilities.currentExtent.width == std::numeric_limits<uint32>::max())
    {
        int32 Width, Height;
        glfwGetFramebufferSize((GLFWwindow*)GetWindow()->GetNativeWindow(), &Width, &Height);

        VkExtent2D Extent = {
            static_cast<uint32>(Width),
            static_cast<uint32>(Height)
        };

        Extent.width = Math::Clamp(Extent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        Extent.height = Math::Clamp(Extent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

        return Extent;
    }
    else
    {
        return Capabilities.currentExtent;
    }
}

void RkVulkanRendererContext::SetupDebugMessenger(VkInstance Instance)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfo();
    VkResult Result = Utils::CreateDebugUtilsMessengerEXT(Instance, &createInfo, nullptr, &DebugMessenger);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS);
}

void RkVulkanRendererContext::DestroyDebugMessenger(VkInstance Instance)
{
    Utils::DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
}

VkDebugUtilsMessengerCreateInfoEXT RkVulkanRendererContext::CreateDebugMessengerCreateInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    return createInfo;
}

VkDebugUtilsMessengerEXT RkVulkanRendererContext::GetDebugMessenger() const
{
    return DebugMessenger;
}

/* RkVulkanValidationLayer */

bool RkValidationLayer::IsSupported()
{
    uint32 LayerCount = 0;
    vkEnumerateInstanceLayerProperties(&LayerCount, 0);

    std::vector<VkLayerProperties> LayerProperties(LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, LayerProperties.data());

    for (const auto& LayerProperty : LayerProperties)
    {
        if (strcmp(Name, LayerProperty.layerName) == 0)
        {
            return true;
        }
    }
    return false;
}


/* CVulkanRendererContext */

void RkVulkanRendererContext::Init()
{
    ValidationLayers.push_back(RkValidationLayer("VK_LAYER_KHRONOS_validation"));

    CreateInstance();
    CreateSurfaceInterface();
    CreatePhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain();
}

void RkVulkanRendererContext::Destroy() 
{
    for (auto ImageView : SwapchainImagesView)
    {
        vkDestroyImageView(LogicalDevice, ImageView, nullptr);
    }

    vkDestroySwapchainKHR(LogicalDevice, Swapchain, nullptr);
    vkDestroySurfaceKHR((VkInstance)ContextHandle, SurfaceInterface, nullptr);
    vkDestroyDevice(LogicalDevice, nullptr);
    DestroyDebugMessenger((VkInstance)ContextHandle);
    vkDestroyInstance((VkInstance)ContextHandle, nullptr);
}

void RkVulkanRendererContext::CreateInstance()
{
    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "Rocket Vulkan Application";
    AppInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    AppInfo.pEngineName = "No Engine";
    AppInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

    // GLFW required Vulkan extensions
    uint32 GlfwExtensionCount = 0;
    const char** GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);

    std::vector<const char*> Extensions(GlfwExtensions, GlfwExtensions + GlfwExtensionCount);
    Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    CreateInfo.enabledExtensionCount = static_cast<uint32>(Extensions.size());
    CreateInfo.ppEnabledExtensionNames = Extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};

    std::vector<const char*> Result;
    for (auto& ValidationLayer : ValidationLayers)
    {
        Result.emplace_back(ValidationLayer.Name);
    }
    CreateInfo.enabledLayerCount = Result.size();
    CreateInfo.ppEnabledLayerNames = Result.data();

    DebugCreateInfo = CreateDebugMessengerCreateInfo();
    CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;

    VkInstance Instance;
    VkResult CreateInstanceResult = vkCreateInstance(&CreateInfo, nullptr, &Instance);
    RK_ENGINE_ASSERT(CreateInstanceResult == VK_SUCCESS, "Failed to create Vulkan context.");
    ContextHandle = Instance;

    SetupDebugMessenger(Instance);
}

void RkVulkanRendererContext::CreateSurfaceInterface()
{
    VkWin32SurfaceCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    CreateInfo.hwnd = glfwGetWin32Window((GLFWwindow*)GetWindow()->GetNativeWindow());
    CreateInfo.hinstance = GetModuleHandle(nullptr);

    VkResult Result;
    Result = glfwCreateWindowSurface((VkInstance)ContextHandle, (GLFWwindow*)GetWindow()->GetNativeWindow(), nullptr, &SurfaceInterface);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Win32 surface");
}

void RkVulkanRendererContext::CreatePhysicalDevice()
{
    uint32 DeviceCount = 0;
    vkEnumeratePhysicalDevices((VkInstance)ContextHandle, &DeviceCount, 0);
    RK_ENGINE_ASSERT(DeviceCount > 0, "No physical device found.");

    std::vector<VkPhysicalDevice> PhysicalDevices(DeviceCount);
    vkEnumeratePhysicalDevices((VkInstance)ContextHandle, &DeviceCount, PhysicalDevices.data());

    for (VkPhysicalDevice Device : PhysicalDevices)
    {
        std::optional<uint32> Indices;
        if (IsVulkanCapableDevice(Device))
        {
            PhysicalDevice = Device;
            break;
        }
    }
}
    

void RkVulkanRendererContext::CreateLogicalDevice()
{
    RkQueueFamilyIndices QueueFamilyIndices = RequestQueueFamilies(PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    std::set<uint32> UniqueQueueFamilies { QueueFamilyIndices.GraphicsFamily.value(), QueueFamilyIndices.PresentFamily.value() };
    
    float QueuePriority = 1.0f;
    for (uint32 Idx : UniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = Idx;
        QueueCreateInfo.queueCount = 1;
        QueueCreateInfo.pQueuePriorities = &QueuePriority;
        QueueCreateInfos.push_back(QueueCreateInfo);
    }

    VkPhysicalDeviceFeatures DeviceFeatures{};

    VkDeviceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.queueCreateInfoCount = static_cast<uint32>(QueueCreateInfos.size());
    CreateInfo.pEnabledFeatures = &DeviceFeatures;
    CreateInfo.enabledExtensionCount = static_cast<uint32>(Extensions.size());
    CreateInfo.ppEnabledExtensionNames = Extensions.data();

    std::vector<const char*> Result;
    for (auto& ValidationLayer : ValidationLayers)
    {
        Result.emplace_back(ValidationLayer.Name);
    }
    CreateInfo.enabledLayerCount = static_cast<uint32>(Result.size());
    CreateInfo.ppEnabledLayerNames = Result.data();

    VkResult CreateLogicalDeviceResult = vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &LogicalDevice);
    RK_ENGINE_ASSERT(CreateLogicalDeviceResult == VK_SUCCESS, "Failed to create logical device.");

    vkGetDeviceQueue(LogicalDevice, QueueFamilyIndices.GraphicsFamily.value(), 0, &GraphicsQueue);
    vkGetDeviceQueue(LogicalDevice, QueueFamilyIndices.PresentFamily.value(), 0, &GraphicsQueue);
}

void RkVulkanRendererContext::CreateSwapchain()
{
    RkSwapChainSupportDetails SwapchainSupport = RequestSwapchainSupportDetails(PhysicalDevice);

    VkSurfaceFormatKHR SurfaceFormat = SelectSwapchainSurfaceFormat(SwapchainSupport.Formats);
    VkPresentModeKHR PresentMode = SelectSwapchainPresentMode(SwapchainSupport.PresentMode);
    VkExtent2D Extent = SelectSwapExtent(SwapchainSupport.Capabilities);

    // Number of images to use in our swapchain
    uint32 ImageCount = SwapchainSupport.Capabilities.minImageCount + 1;
    if (SwapchainSupport.Capabilities.maxImageCount > 0 && ImageCount > SwapchainSupport.Capabilities.maxImageCount)
    {
        ImageCount = SwapchainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = SurfaceInterface;
    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = Extent;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    RkQueueFamilyIndices Indices = RequestQueueFamilies(PhysicalDevice);
    uint32 QueueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    if (Indices.GraphicsFamily != Indices.PresentFamily)
    {
        // Images can be used across multiple queue families without explicit ownership transfers.
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else
    {
        // An image is owned by one queue family at a time and ownership must be explicitly 
        // transferred before using it in another queue family. This option offers the best performance.
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    CreateInfo.preTransform = SwapchainSupport.Capabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE;
    CreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult Result = vkCreateSwapchainKHR(LogicalDevice, &CreateInfo, nullptr, &Swapchain);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create swapchain.");

    vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &ImageCount, nullptr);
    SwapchainImages.resize(ImageCount);
    vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &ImageCount, SwapchainImages.data());

    SwapchainImageFormat = SurfaceFormat.format;
    SwapchainExtent = Extent;

    // To use any of the images in the swapchain, a VkImageView object is required (a readonly view into the image).
    SwapchainImagesView.resize(ImageCount);
    for (size_t Idx = 0; Idx < ImageCount; Idx++)
    {
        VkImageViewCreateInfo ImageViewCreateInfo{};
        ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewCreateInfo.image = SwapchainImages[Idx];
        ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // Specifies how an image should be interpreted (eg. 1D textures, 2D textures, 3D textures and cube maps).
        ImageViewCreateInfo.format = SwapchainImageFormat;

        // Default color channel mapping
        ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Defines image purpose and what part of the image should be accessed. 
        // Image is currently set to be used as color targets without any mipmapping levels or multiple layers.
        ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageViewCreateInfo.subresourceRange.baseMipLevel= 0;
        ImageViewCreateInfo.subresourceRange.levelCount = 1;
        ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ImageViewCreateInfo.subresourceRange.layerCount = 1;

        VkResult Result = vkCreateImageView(LogicalDevice, &ImageViewCreateInfo, nullptr, &SwapchainImagesView[Idx]);
        RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");
    }
}
 
RkSwapChainSupportDetails RkVulkanRendererContext::RequestSwapchainSupportDetails(VkPhysicalDevice PhysicalDevice)
{
    RkSwapChainSupportDetails SwapChainSupportDetails;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, SurfaceInterface, &SwapChainSupportDetails.Capabilities);

    uint32 FormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, SurfaceInterface, &FormatCount, nullptr);

    if (FormatCount != 0)
    {
        SwapChainSupportDetails.Formats.resize(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, SurfaceInterface, &FormatCount, SwapChainSupportDetails.Formats.data());
    }

    uint32 PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, SurfaceInterface, &PresentModeCount, SwapChainSupportDetails.PresentMode.data());
    if (PresentModeCount != 0)
    {
        SwapChainSupportDetails.PresentMode.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, SurfaceInterface, &PresentModeCount, SwapChainSupportDetails.PresentMode.data());
    }
    return SwapChainSupportDetails;
}

RkQueueFamilyIndices RkVulkanRendererContext::RequestQueueFamilies(VkPhysicalDevice PhysicalDevice)
{
    RkQueueFamilyIndices Indices;

    uint32 QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamilies.data());

    for (int32 Index = 0; Index < QueueFamilyCount; Index++)
    {
        const VkQueueFamilyProperties& FamilyProperty = QueueFamilies[Index];
        if (FamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            Indices.GraphicsFamily = Index;
        }

        VkBool32 PresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, Index, SurfaceInterface, &PresentSupport);

        if (PresentSupport)
        {
            Indices.PresentFamily = Index;
        }

        if (Indices.IsComplete())
        {
            break;
        }
    }
    return Indices;
}

// isDeviceSuitable + checkDeviceExtensionSupport
bool RkVulkanRendererContext::IsVulkanCapableDevice(VkPhysicalDevice PhysicalDevice)
{
    RkQueueFamilyIndices Indices = RequestQueueFamilies(PhysicalDevice);

    uint32 ExtensionCount;
    vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, nullptr);

    std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
    vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

    std::set<std::string> RequiredExtensions(Extensions.begin(), Extensions.end());
    for (const auto& Extension : AvailableExtensions)
    {
        RequiredExtensions.erase(Extension.extensionName);
    }
    
    bool bExtensionSupport = RequiredExtensions.empty();
    bool bSwapChainAdequate = false;
    
    if (bExtensionSupport)
    {
        RkSwapChainSupportDetails SwapChainSupport = RequestSwapchainSupportDetails(PhysicalDevice);
        bSwapChainAdequate = !SwapChainSupport.Formats.empty() && !SwapChainSupport.PresentMode.empty();
    }

    return Indices.IsComplete() && bExtensionSupport && bSwapChainAdequate;
}
