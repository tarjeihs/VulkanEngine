#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <optional>

#include "Renderer/RendererContext.h"
#include "Math/MathTypes.h"
#include "Core/Assert.h"

//enum ERenderPipelineType
//{
//    // A standard pipeline for rendering directly to the screen.
//    ForwardRenderingPipeline,
//
//    // A pipeline that separates geometry processing from lighting calculations to improve performance in scenes with many lights.
//    DeferredRenderingPipeline,
//
//    // A pipeline for applying effects like bloom, tone mapping, and anti - aliasing after the main rendering pass.
//    PostProcessingPipeline,
//
//    // A pipeline dedicated to rendering shadow maps for shadow calculations.
//    ShadowMappingPipeline
//};

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

// With 2 frames in flight, the CPU and the GPU can be working on their own tasks at the same time. 
// If the CPU finishes early, it will wait till the GPU finishes rendering before submitting more work. 
// With 3 or more frames in flight, the CPU could get ahead of the GPU, adding frames of latency
static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

 /*   
  *   Shader stages : the shader modules that define the functionality of the programmable stages of the graphics pipeline
  *   Fixed - function state : all of the structures that define the fixed - function stages of the pipeline, like input assembly, rasterizer, viewport and color blending
  *   Pipeline layout : the uniform and push values referenced by the shader that can be updated at draw time
  *   Render pass : the attachments referenced by the pipeline stages and their usage
  *
  *   All of these combined fully define the functionality of the graphics pipeline
  **/
class RkVulkanRendererContext : public CRendererContext
{    
public:
    virtual void Init() override;
    virtual void Destroy() override;

    inline VkPhysicalDevice GetPhysicalDevice() const { return PhysicalDevice; }
    inline VkDevice GetLogicalDevice() const { return LogicalDevice; }

    // Records a single draw call to command buffer
    void Record(VkCommandBuffer CommandBuffer, uint32 ImageIndex);

    // Submits the recorded command buffer
    void Draw();

    // Recreates the entire swapchain (swapchains, framebuffers, image views)
    void RegenerateSwapchain();

    // Loops until window regains focus
    void AwaitFocus();

protected:
    void CreateInstance();
    void CreateSurfaceInterface();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void CreateRenderPass();
    void CreateRenderPipeline();
    void CreateFramebuffers();
    void CreateCommandPoolAndBuffer();
    void CreateSynchronizationObjects();


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
    VkPhysicalDevice PhysicalDevice;                                                // GPU
    VkDevice LogicalDevice;                                                         // Driver

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;
    VkSurfaceKHR SurfaceInterface;
    VkDebugUtilsMessengerEXT DebugMessenger;

    VkRenderPass RenderPass;                                                        // Describes what attachments, subpasses and dependencies to use on the framebuffer.

    VkSwapchainKHR Swapchain;                                                       // Image 1: (On Display): Image is currently being shown on the screen. Image 2: (In the pipeline): GPU is currently rendering onto this image. Image 3: (Waiting) Image is ready and waiting.
    VkFormat SwapchainImageFormat;                                                  // Defines pixel format of the images in the swapchain. (Color format and Depth/Stencil format)
    VkExtent2D SwapchainExtent;                                                     // Defines the width and height (in pixels) of the images in the swapchain.

    VkPipelineLayout PipelineLayout;                                                // Connects the inputs a shader needs (like uniforms, push constants and descriptor sets) to the actual data sources.
    VkPipeline Pipeline;                                                            // A compiled (including shader stages, fixed-function stages, state objects, and pipeline layout) version of the shader code, ready to be executed by GPU.

    size_t CurrentFrame = 0;
    VkCommandPool CommandPool;                                                      // Manages memory that is used to store the buffers and allocates command buffers.
    std::vector<VkFence> InFlightFences;                                            // Wait for this fence before submitting new commands, reset the fence, and submit commands with the fence to be signaled when done.
    std::vector<VkSemaphore> ImageAvailableSemaphores;                              // Signal the semaphore after acquiring an image and wait on it before rendering.
    std::vector<VkSemaphore> RenderFinishedSemaphores;                              // Signal the semaphore after rendering is complete and wait on it before presenting the image.
    std::vector<VkCommandBuffer> CommandBuffers;
    
    std::vector<VkRenderPass> RenderPasses;
    std::vector<VkImage> SwapchainImages;           
    std::vector<VkFramebuffer> SwapchainFramebuffers;                               // Collection of image views used as attachments in ther render pass. Stores information such as RGBA(uint8) per pixel, Depth(float), Stencil(uint8)
    std::vector<VkImageView> SwapchainImageViews;

    std::vector<RkValidationLayer> ValidationLayers;
    std::vector<const char*> Extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};