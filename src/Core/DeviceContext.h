#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "DeviceSelector.h"
#include "Window.h" // 需要知道窗口信息
#include "vk_mem_alloc.h"
#include <memory>

class FDeviceContext
{
public:
    // 初始化需要窗口，因为 Surface 的创建依赖窗口
    FDeviceContext(FWindow& WindowObj);
    ~FDeviceContext();

    VkPhysicalDevice GetPhysicalDevice() const { return PhysicalDevice; }
    VkDevice GetLogicalDevice() const { return LogicalDevice; }
    
    VmaAllocator GetAllocator() const { return Allocator; }

    VkQueue GetGraphicsQueue() const { return GraphicsQueue; }
    VkQueue GetPresentQueue() const { return PresentQueue; }
    VkQueue GetComputeQueue() const { return ComputeQueue; }

    VkSurfaceKHR GetSurface() const { return Surface; }

private:
    void CreateInstance();       // 步骤 1
    void SetupDebugMessenger();  // 步骤 2 (Debug)
    void CreateSurface();
    void PickPhysicalDevice();   // 步骤 3
    void CreateLogicalDevice();  // 步骤 4

    void CreateAllocator();
    
    // 引用窗口
    FWindow& WindowRef;
    // Vulkan 句柄
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