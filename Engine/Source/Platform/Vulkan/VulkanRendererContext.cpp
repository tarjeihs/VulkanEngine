#include "VulkanRendererContext.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32

#include <cassert>
#include <stdexcept>
#include <set>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan_core.h>
#include <algorithm>  // for std::transform

#include "Core/Assert.h"
#include "Core/Window.h"
#include "Math/Math.h"
#include "Renderer/Shader.h"

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
    CreateRenderPass();
    CreateRenderPipeline();
    CreateFramebuffers();
    CreateCommandPoolAndBuffer();
    CreateSynchronizationObjects();
}

void RkVulkanRendererContext::Destroy() 
{
    // Wait until safe to access async
    vkDeviceWaitIdle(GetLogicalDevice());

    for (auto Framebuffer : SwapchainFramebuffers)
    {
        vkDestroyFramebuffer(GetLogicalDevice(), Framebuffer, nullptr);
    }

    for (auto ImageView : SwapchainImageViews)
    {
        vkDestroyImageView(LogicalDevice, ImageView, nullptr);
    }
    
    vkDestroySwapchainKHR(LogicalDevice, Swapchain, nullptr);
    vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);
    vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, nullptr);

    for (size_t Index = 0; Index < MAX_FRAMES_IN_FLIGHT; Index++)
    {
        vkDestroySemaphore(GetLogicalDevice(), RenderFinishedSemaphores[Index], nullptr);
        vkDestroySemaphore(GetLogicalDevice(), ImageAvailableSemaphores[Index], nullptr);
        vkDestroyFence(GetLogicalDevice(), InFlightFences[Index], nullptr);
    }

    vkDestroyCommandPool(GetLogicalDevice(), CommandPool, nullptr);
    vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);
    vkDestroyDevice(LogicalDevice, nullptr);
    vkDestroySurfaceKHR((VkInstance)ContextHandle, SurfaceInterface, nullptr);
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
    vkGetDeviceQueue(LogicalDevice, QueueFamilyIndices.PresentFamily.value(), 0, &PresentQueue);
}

void RkVulkanRendererContext::CreateRenderPass()
{
    VkAttachmentDescription ColorAttachment{};
    ColorAttachment.format = SwapchainImageFormat;
    ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ColorAttachmentRef{};
    ColorAttachmentRef.attachment = 0;
    ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription Subpass{};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpass.colorAttachmentCount = 1;
    Subpass.pColorAttachments = &ColorAttachmentRef;

    VkSubpassDependency Dependency{};
    Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass = 0;
    Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.srcAccessMask = 0;
    Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    CreateInfo.attachmentCount = 1;
    CreateInfo.pAttachments = &ColorAttachment;
    CreateInfo.subpassCount = 1;
    CreateInfo.pSubpasses = &Subpass;
    CreateInfo.dependencyCount = 1;
    CreateInfo.pDependencies = &Dependency;

    VkResult Result = vkCreateRenderPass(GetLogicalDevice(), &CreateInfo, nullptr, &RenderPass);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create render pass.");
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
    SwapchainImageViews.resize(ImageCount);
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

        VkResult Result = vkCreateImageView(LogicalDevice, &ImageViewCreateInfo, nullptr, &SwapchainImageViews[Idx]);
        RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");
    }
}

void RkVulkanRendererContext::CreateRenderPipeline()
{
    // Configure the pipeline states explicitly as it will be baked into an immutable pipeline state object. 

    // Format of vertex data passed into vertex shader
    VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo{};
    VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    VertexInputCreateInfo.vertexAttributeDescriptionCount = 0;

    // NOTE: Without an index buffer(IBO), we cannot perform optimizations like reusing vertices.
    // Geometry topology and primitive restart
    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo{};
    InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Triangle from every 3 vertices without reuse
    InputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    // Describes the region of the framebuffer that the output will be rendered to.
    VkPipelineViewportStateCreateInfo ViewportCreateInfo{};
    ViewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportCreateInfo.viewportCount = 1;
    ViewportCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo{};
    RasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizerCreateInfo.depthClampEnable = VK_FALSE;
    RasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // Determines how fragments are generated on the geometry. VK_POLYGON_MODE_FILL will fill the area of the polygon with fragments.
    RasterizerCreateInfo.lineWidth = 1.0f; // Thickness of lines in terms of number of fragments.
    RasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; // Vertex order for faces to be considered front-facing.
    RasterizerCreateInfo.depthBiasEnable = VK_FALSE;

    // Anti-aliasing
    VkPipelineMultisampleStateCreateInfo MultisampleCreateInfo{};
    MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    MultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer.
    // Blending the new color with the old color based on its opacity. (Alpha blending)
    VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_TRUE;
    ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo ColorBlendCreateInfo{};
    ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendCreateInfo.logicOpEnable = VK_FALSE;
    ColorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendCreateInfo.attachmentCount = 1;
    ColorBlendCreateInfo.pAttachments = &ColorBlendAttachment;
    ColorBlendCreateInfo.blendConstants[0] = 0.0f;
    ColorBlendCreateInfo.blendConstants[1] = 0.0f;
    ColorBlendCreateInfo.blendConstants[2] = 0.0f;
    ColorBlendCreateInfo.blendConstants[3] = 0.0f;

    const std::vector<VkDynamicState> DynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo{};
    DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32>(DynamicStates.size());
    DynamicStateCreateInfo.pDynamicStates = DynamicStates.data();

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo{};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.setLayoutCount = 0;
    PipelineLayoutCreateInfo.pushConstantRangeCount = 0;

    VkResult Result = vkCreatePipelineLayout(GetLogicalDevice(), &PipelineLayoutCreateInfo, nullptr, &PipelineLayout);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan pipeline layout.");

    // TODO: This is just for testing!
    RkShader Shader;
    Shader.Compile(RK_SHADERTYPE_VERTEXSHADER, L"../Shaders/SimpleShaderVert.hlsl", L"main", "vs_6_0");
    Shader.Compile(RK_SHADERTYPE_FRAGMENTSHADER, L"../Shaders/SimpleShaderFrag.hlsl", L"main", "ps_6_0");

    // Copy shader create info seperate vector
    std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfos(Shader.ShaderPrograms.size());
    std::transform(Shader.ShaderPrograms.begin(), Shader.ShaderPrograms.end(), ShaderStageCreateInfos.begin(), [](const SShaderProgram& shaderProgram)
    {
        return shaderProgram.CreateInfo;
    });

    // Pipeline configuration
    VkGraphicsPipelineCreateInfo PipelineCreateInfo{};
    PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCreateInfo.stageCount = static_cast<uint32>(ShaderStageCreateInfos.size());
    PipelineCreateInfo.pStages = ShaderStageCreateInfos.data();
    PipelineCreateInfo.pVertexInputState = &VertexInputCreateInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
    PipelineCreateInfo.pViewportState = &ViewportCreateInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizerCreateInfo;
    PipelineCreateInfo.pMultisampleState = &MultisampleCreateInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendCreateInfo;
    PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
    PipelineCreateInfo.layout = PipelineLayout;
    PipelineCreateInfo.renderPass = RenderPass;
    PipelineCreateInfo.subpass = 0;
    PipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    Result = vkCreateGraphicsPipelines(GetLogicalDevice(), VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &Pipeline);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan pipeline.");

    // Perform Shader Module destruction and cleanup
    Shader.PostCompile();
}

void RkVulkanRendererContext::CreateFramebuffers()
{
    SwapchainFramebuffers.resize(SwapchainImageViews.size());

    for (size_t Index = 0; Index < SwapchainImageViews.size(); Index++)
    {
        VkImageView Attachments[] = { SwapchainImageViews[Index] };

        VkFramebufferCreateInfo CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        CreateInfo.renderPass = RenderPass; // Specify what render pass to use
        CreateInfo.attachmentCount = 1;
        CreateInfo.pAttachments = Attachments;
        CreateInfo.width = SwapchainExtent.width;
        CreateInfo.height = SwapchainExtent.height;
        CreateInfo.layers = 1;

        VkResult Result = vkCreateFramebuffer(GetLogicalDevice(), &CreateInfo, nullptr, &SwapchainFramebuffers[Index]);
        RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create Vulkan framebuffer.");
    }
}

void RkVulkanRendererContext::CreateCommandPoolAndBuffer()
{
    RkQueueFamilyIndices QueueFamilyIndices = RequestQueueFamilies(GetPhysicalDevice());

    // We will be recording a command buffer every frame, so we want to be able to reset and rerecord over it.
    // Thus, we need to set the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag bit for our command pool.

    // Command buffers are executed by submitting them on one of the device queues, like the graphics and presentation queues we retrieved.
    // Each command pool can only allocate command buffers that are submitted on a single type of queue.

    // Create the command pool
    VkCommandPoolCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffers to be rerecorded individually
    CreateInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsFamily.value();

    VkResult Result = vkCreateCommandPool(GetLogicalDevice(), &CreateInfo, nullptr, &CommandPool);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create command pool.");

    // Allocate multiple command buffers
    CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Can be submitted to a queue for execution
    AllocInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());

    Result = vkAllocateCommandBuffers(GetLogicalDevice(), &AllocInfo, CommandBuffers.data());
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to allocate command buffers from command pool.");
}

void RkVulkanRendererContext::CreateSynchronizationObjects()
{
    InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo SemaphoreCreateInfo{};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceCreateInfo{};
    FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult Result = vkCreateSemaphore(GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &ImageAvailableSemaphores[i]);
        RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create synchronization object for one frame.");

        Result = vkCreateSemaphore(GetLogicalDevice(), &SemaphoreCreateInfo, nullptr, &RenderFinishedSemaphores[i]);
        RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create synchronization object for one frame.");

        Result = vkCreateFence(GetLogicalDevice(), &FenceCreateInfo, nullptr, &InFlightFences[i]);
        RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to create synchronization object for one frame.");
    }
}

void RkVulkanRendererContext::RegenerateSwapchain()
{
    vkDeviceWaitIdle(GetLogicalDevice());

    while (GetWindow()->IsMinimized())
    {
        // Rough handling of window minimization.
        glfwWaitEvents();
    }

    for (size_t i = 0; i < SwapchainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(GetLogicalDevice(), SwapchainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < SwapchainImageViews.size(); i++)
    {
        vkDestroyImageView(GetLogicalDevice(), SwapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(GetLogicalDevice(), Swapchain, nullptr);

    CreateSwapchain();
    CreateFramebuffers();
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

void RkVulkanRendererContext::Record(VkCommandBuffer CommandBuffer, uint32 ImageIndex)
{
    VkCommandBufferBeginInfo BufferBeginInfo{};
    BufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BufferBeginInfo.flags = 0;
    BufferBeginInfo.pInheritanceInfo = nullptr;

    VkResult Result = vkBeginCommandBuffer(CommandBuffer, &BufferBeginInfo);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to allocate command buffer from command pool.");

    VkRenderPassBeginInfo RenderPassBeginInfo{};
    RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassBeginInfo.renderPass = RenderPass;
    RenderPassBeginInfo.framebuffer = SwapchainFramebuffers[ImageIndex];
    RenderPassBeginInfo.renderArea.offset = { 0, 0 };
    RenderPassBeginInfo.renderArea.extent = SwapchainExtent;

    VkClearValue ClearColor = { { { 0.01f, 0.01f, 0.01f, 1.0f } } };
    RenderPassBeginInfo.clearValueCount = 1;
    RenderPassBeginInfo.pClearValues = &ClearColor;

    vkCmdBeginRenderPass(CommandBuffer, &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);

    VkViewport Viewport{};
    Viewport.x = 0.0f;
    Viewport.y = 0.0f;
    Viewport.width = (float)SwapchainExtent.width;
    Viewport.height = (float)SwapchainExtent.height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;
    vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);

    VkRect2D Scissor{};
    Scissor.offset = { 0, 0 };
    Scissor.extent = SwapchainExtent;
    vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);

    vkCmdDraw(CommandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(CommandBuffer);

    Result = vkEndCommandBuffer(CommandBuffer);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to record command buffer.");
}

void RkVulkanRendererContext::Draw()
{
    vkWaitForFences(GetLogicalDevice(), 1, &InFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t ImageIndex = 0;
    VkResult Result = vkAcquireNextImageKHR(GetLogicalDevice(), Swapchain, UINT64_MAX, ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);
    if (Result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RegenerateSwapchain();
        return;
    }
    
    vkResetFences(GetLogicalDevice(), 1, &InFlightFences[CurrentFrame]);

    vkResetCommandBuffer(CommandBuffers[CurrentFrame], /* VkCommandBufferResetFlagBits*/ 0);
    Record(CommandBuffers[CurrentFrame], ImageIndex);

    VkSemaphore SignalSemaphores[] = { RenderFinishedSemaphores[CurrentFrame] };
    VkSemaphore WaitSemaphores[] = { ImageAvailableSemaphores[CurrentFrame] };
    VkPipelineStageFlags WaitFlags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSwapchainKHR Swapchains[] = { Swapchain };

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitFlags;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffers[CurrentFrame];
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    // Enqueue 
    Result = vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, InFlightFences[CurrentFrame]);
    RK_ENGINE_ASSERT(Result == VK_SUCCESS, "Failed to submit draw command buffer.");

    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = SignalSemaphores;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = Swapchains;
    PresentInfo.pImageIndices = &ImageIndex;

    Result = vkQueuePresentKHR(PresentQueue, &PresentInfo);
    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        RegenerateSwapchain();
    }

    CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
