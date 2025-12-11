#pragma once
#include "DeviceSelector.h"
#include "Window.h"
#include "vk_mem_alloc.h"
#include "Swapchain.h"

class FDeviceContext
{
public:
    FDeviceContext(FWindow& WindowObj);
    ~FDeviceContext();

    VkPhysicalDevice GetPhysicalDevice() const { check(PhysicalDevice != VK_NULL_HANDLE); return PhysicalDevice; }
    VkDevice GetLogicalDevice() const { check(LogicalDevice != VK_NULL_HANDLE); return LogicalDevice; }
    
    VmaAllocator GetAllocator() const { check(Allocator != VK_NULL_HANDLE); return Allocator; }

    VkQueue GetGraphicsQueue() const { check(GraphicsQueue != VK_NULL_HANDLE); return GraphicsQueue; }
    VkQueue GetPresentQueue() const { check(PresentQueue != VK_NULL_HANDLE); return PresentQueue; }
    VkQueue GetComputeQueue() const { check(ComputeQueue != VK_NULL_HANDLE); return ComputeQueue; }

    VkSurfaceKHR GetSurface() const { check(Surface != VK_NULL_HANDLE); return Surface; }

    FQueueFamilyIndices GetQueueFamilyIndices() const { return QueueIndices; }

    FSwapchain& GetSwapchain() const { check(Swapchain); return *Swapchain; }

    void Init();

    void RecreateSwapchain();

    bool RenderFrame();

private:
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();

    void CreateAllocator();
    void TestVMA();

    VkShaderModule CreateShaderModule(const std::vector<char>& InCode) const;
    void CreatePipelineLayout();

    void CreateGraphicsPipeline();

    void CreateCommandPool();
    void CreateCommandBuffers();

    void RecordCommandBuffers(VkCommandBuffer InCommandBuffer, uint32_t InImageIndex);
    void CreateSyncObjects();

    FWindow& WindowRef;

    VkInstance Instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice LogicalDevice = VK_NULL_HANDLE;

    FQueueFamilyIndices QueueIndices;
    VkQueue GraphicsQueue = VK_NULL_HANDLE;
    VkQueue PresentQueue = VK_NULL_HANDLE;
    VkQueue ComputeQueue = VK_NULL_HANDLE;

    VmaAllocator Allocator = VK_NULL_HANDLE;

    std::unique_ptr<class FSwapchain> Swapchain;

    VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;
    VkPipeline GraphicsPipeline = VK_NULL_HANDLE;
    VkCommandPool CommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> CommandBuffers;

    std::vector<VkSemaphore> ImageAvailableSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;
    std::vector<VkFence> InFlightFences;

    int CurrentFrame = 0;
};