#pragma once
#include "DeviceSelector.h"
#include "Window.h" // 需要知道窗口信息
#include "vk_mem_alloc.h"

class FDeviceContext
{
public:
    // 初始化需要窗口，因为 Surface 的创建依赖窗口
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

private:
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();

    void CreateAllocator();
    
    FWindow& WindowRef;

    VkInstance Instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE; // 验证层
    VkSurfaceKHR Surface = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE; // 物理设备（不需要 Destroy）
    VkDevice LogicalDevice = VK_NULL_HANDLE; // 逻辑设备

    FQueueFamilyIndices QueueIndices;
    VkQueue GraphicsQueue = VK_NULL_HANDLE;
    VkQueue PresentQueue = VK_NULL_HANDLE;
    VkQueue ComputeQueue = VK_NULL_HANDLE;

    VmaAllocator Allocator = VK_NULL_HANDLE;

    std::unique_ptr<class FSwapchain> Swapchain;
};