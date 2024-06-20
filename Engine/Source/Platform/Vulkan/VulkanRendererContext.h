#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <optional>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <array>

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

struct RkVertex
{
    glm::vec2 Position;
    glm::vec3 Color;

    static VkVertexInputBindingDescription DescribeBindingDescription()
    {
        VkVertexInputBindingDescription BindingDescription{};
        BindingDescription.binding = 0;
        BindingDescription.stride = sizeof(RkVertex);
        BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // Move to the next data entry after each vertex

        return BindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> DescribeAttributeDescription()
    {
        std::array<VkVertexInputAttributeDescription, 2> AttributeDescriptions;

        AttributeDescriptions[0].binding = 0;
        AttributeDescriptions[0].location = 0;
        AttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescriptions[0].offset = offsetof(RkVertex, Position);

        AttributeDescriptions[1].binding = 0;
        AttributeDescriptions[1].location = 1;
        AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescriptions[1].offset = offsetof(RkVertex, Color);

        return AttributeDescriptions;
    }
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

protected:
    void CreateInstance();
    void CreateSurfaceInterface();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapchain();
    void CreateRenderPass();
    void CreateRenderPipeline();
    void CreateFramebuffers();
    void CreateBuffers();
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

    uint32 FindMemoryType(uint32 TypeFilter, VkMemoryPropertyFlags Properties);

private:
    VkPhysicalDevice PhysicalDevice;                                                // GPU
    VkDevice LogicalDevice;                                                         // Driver

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;
    VkSurfaceKHR SurfaceInterface;
    VkDebugUtilsMessengerEXT DebugMessenger;
    
    VkBuffer VertexBuffer;
    VkDeviceMemory VertexBufferMemory;

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