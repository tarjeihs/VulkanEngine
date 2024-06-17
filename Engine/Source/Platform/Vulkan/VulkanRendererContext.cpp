#include "VulkanRendererContext.h"

#include <cassert>
#include <stdexcept>
#include <glfw/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "Core/Assert.h"
#include "Math/MathTypes.h"

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
        if (strcmp(ValidationLayerName, LayerProperty.layerName) == 0)
        {
            return true;
        }
    }

    return false;
}

void RkVulkanValidationLayer::Enable(VkInstanceCreateInfo& CreateInfo)
{
    assert(IsSupported(), "This Validation Layer is not supported.");

    const char** newLayerNames = static_cast<const char**>(std::malloc((CreateInfo.enabledLayerCount + 1) * sizeof(const char*)));

    if (CreateInfo.ppEnabledLayerNames)
    {
        std::memcpy(newLayerNames, CreateInfo.ppEnabledLayerNames, CreateInfo.enabledLayerCount * sizeof(const char*));
    }

    newLayerNames[CreateInfo.enabledLayerCount] = ValidationLayerName;

    CreateInfo.enabledLayerCount++;
    CreateInfo.ppEnabledLayerNames = newLayerNames;
}

/* CVulkanRendererContext */

void CVulkanRendererContext::Init()
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
    RkVulkanValidationLayer* ValidationLayer = new RkVulkanValidationLayer("VK_LAYER_KHRONOS_validation");
    ValidationLayer->Enable(CreateInfo);

    DebugMessenger = std::make_shared<RkVulkanDebugMessenger>();
    DebugCreateInfo = DebugMessenger->CreateDebugMessengerCreateInfo();
    CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;

    VkInstance Instance;
    VkResult Result = vkCreateInstance(&CreateInfo, nullptr, &Instance);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan context.");
    ContextHandle = Instance;

    DebugMessenger->SetupDebugMessenger(Instance);
}

void CVulkanRendererContext::Destroy() 
{
    DebugMessenger->DestroyDebugMessenger((VkInstance)ContextHandle);

    vkDestroyInstance((VkInstance)ContextHandle, nullptr);
}