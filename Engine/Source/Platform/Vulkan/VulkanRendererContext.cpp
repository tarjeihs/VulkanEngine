#include "VulkanRendererContext.h"

#include <cassert>
#include <stdexcept>
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_core.h>

#include "Core/Assert.h"
#include "Core/Window.h"

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

    RkQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice PhysicalDevice)
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

            if (Indices.GraphicsFamily.has_value())
            {
                break;
            }
        }
        return Indices;
    }

    bool IsVulkanCapablePhysicalDevice(VkPhysicalDevice PhysicalDevice)
    {
        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperties);

        VkPhysicalDeviceFeatures DeviceFeatures;
        vkGetPhysicalDeviceFeatures(PhysicalDevice, &DeviceFeatures);

        return DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            && DeviceFeatures.geometryShader;
    }
}

/* RkVulkanDebugMessenger */

VKAPI_ATTR VkBool32 VKAPI_CALL RkVulkanDebugMessenger::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
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

void RkVulkanDebugMessenger::SetupDebugMessenger(VkInstance Instance)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfo();
    VkResult Result = Utils::CreateDebugUtilsMessengerEXT(Instance, &createInfo, nullptr, &DebugMessenger);
    assert(Result == VK_SUCCESS);
}

void RkVulkanDebugMessenger::DestroyDebugMessenger(VkInstance Instance)
{
    Utils::DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
}

VkDebugUtilsMessengerCreateInfoEXT RkVulkanDebugMessenger::CreateDebugMessengerCreateInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
    return createInfo;
}

VkDebugUtilsMessengerEXT RkVulkanDebugMessenger::GetDebugMessenger() const
{
    return DebugMessenger;
}

/* RkVulkanValidationLayer */

bool RkVulkanValidationLayer::IsSupported()
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
    ValidationLayers.push_back(RkVulkanValidationLayer("VK_LAYER_KHRONOS_validation"));

    CreateVulkanInstance();
    CreateWindowSurface();
    CreatePhysicalDevice();
    CreateLogicalDevice();
}

void RkVulkanRendererContext::Destroy() 
{
    vkDestroySurfaceKHR((VkInstance)ContextHandle, Surface, nullptr);
    vkDestroyDevice(LogicalDevice, nullptr);
    DebugMessenger->DestroyDebugMessenger((VkInstance)ContextHandle);
    vkDestroyInstance((VkInstance)ContextHandle, nullptr);
}

void RkVulkanRendererContext::CreateVulkanInstance()
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

    DebugMessenger = std::make_shared<RkVulkanDebugMessenger>();
    DebugCreateInfo = DebugMessenger->CreateDebugMessengerCreateInfo();
    CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;

    VkInstance Instance;
    VkResult CreateInstanceResult = vkCreateInstance(&CreateInfo, nullptr, &Instance);
    RK_ENGINE_ASSERT(CreateInstanceResult == VK_SUCCESS, "Failed to create Vulkan context.");
    ContextHandle = Instance;

    DebugMessenger->SetupDebugMessenger(Instance);
}

void RkVulkanRendererContext::CreateWindowSurface()
{
    VkWin32SurfaceCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    CreateInfo.hwnd = glfwGetWin32Window((GLFWwindow*)GetWindow()->GetNativeWindow());
    CreateInfo.hinstance = GetModuleHandle(nullptr);

    VkResult Result;
    Result = glfwCreateWindowSurface((VkInstance)ContextHandle, (GLFWwindow*)GetWindow()->GetNativeWindow(), nullptr, &Surface);
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
        if (Utils::IsVulkanCapablePhysicalDevice(Device))
        {
            PhysicalDevice = Device;
            break;
        }
    }
}

void RkVulkanRendererContext::CreateLogicalDevice()
{
    RkQueueFamilyIndices QueueFamily = Utils::FindQueueFamilies(PhysicalDevice);

    VkDeviceQueueCreateInfo QueueCreateInfo{};
    QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QueueCreateInfo.queueFamilyIndex = QueueFamily.GraphicsFamily.value();
    QueueCreateInfo.queueCount = 1;

    float QueuePriority = 1.0f;
    QueueCreateInfo.pQueuePriorities = &QueuePriority;

    VkPhysicalDeviceFeatures DeviceFeatures{};

    VkDeviceCreateInfo LogicalDeviceCreateInfo{};
    LogicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    LogicalDeviceCreateInfo.pQueueCreateInfos = &QueueCreateInfo;
    LogicalDeviceCreateInfo.queueCreateInfoCount = 1;
    LogicalDeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
    LogicalDeviceCreateInfo.enabledExtensionCount = 0;

    std::vector<const char*> Result;
    for (auto& ValidationLayer : ValidationLayers)
    {
        Result.emplace_back(ValidationLayer.Name);
    }
    LogicalDeviceCreateInfo.enabledLayerCount = Result.size();
    LogicalDeviceCreateInfo.ppEnabledLayerNames = Result.data();

    VkResult CreateLogicalDeviceResult = vkCreateDevice(PhysicalDevice, &LogicalDeviceCreateInfo, nullptr, &LogicalDevice);
    RK_ENGINE_ASSERT(CreateLogicalDeviceResult == VK_SUCCESS, "Failed to create logical device.");

    vkGetDeviceQueue(LogicalDevice, QueueFamily.GraphicsFamily.value(), 0, &GraphicsQueue);
}
 