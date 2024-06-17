#include "VulkanRendererContext.h"

#include <cassert>
#include <stdexcept>
#include <glfw/glfw3.h>
#include <vulkan/vulkan_core.h>

#include "Math/MathTypes.h"

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

    CreateInfo.enabledExtensionCount = GlfwExtensionCount;
    CreateInfo.ppEnabledExtensionNames = GlfwExtensions;

    CreateInfo.enabledLayerCount = 0;

    VkInstance Instance;
    if (vkCreateInstance(&CreateInfo, nullptr, &Instance) != VK_SUCCESS)
    {
        throw new std::runtime_error("Failed to create Vulkan context.");
    }

    ContextHandle = Instance;
}

void CVulkanRendererContext::Destroy()
{
    vkDestroyInstance((VkInstance)ContextHandle, nullptr);    
}
