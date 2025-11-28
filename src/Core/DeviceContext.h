#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Window.h" // 需要知道窗口信息

class DeviceContext
{
public:
    // 初始化需要窗口，因为 Surface 的创建依赖窗口
    DeviceContext(Window& window);
    ~DeviceContext();

    VkDevice getLogicalDevice() const { return m_device; }
    // ... 其他 getter

private:
    void createInstance();       // 步骤 1
    void setupDebugMessenger();  // 步骤 2 (Debug)
    void createSurface();
    void pickPhysicalDevice();   // 步骤 3
    void createLogicalDevice();  // 步骤 4

	bool checkValidationLayerSupport();

    // Vulkan 句柄
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE; // 验证层
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE; // 物理设备（不需要 Destroy）
    VkDevice m_device = VK_NULL_HANDLE; // 逻辑设备

    // 引用窗口
    Window& m_windowRef;
};