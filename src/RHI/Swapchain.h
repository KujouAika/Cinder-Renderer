#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <span>

// 前置声明
class FDeviceContext;
class FWindow;

// 辅助结构体：一次性打包所有查询到的支持信息
struct FSwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities;      // 基础能力（宽高限制、图片数量限制）
    std::vector<VkSurfaceFormatKHR> Formats;    // 支持的像素格式（SRGB, Linear...）
    std::vector<VkPresentModeKHR> PresentModes; // 支持的呈现模式（FIFO, Mailbox...）
};

class FSwapchain
{
public:
    // 构造函数需要持有 Context 和 Window 的引用
    FSwapchain(FDeviceContext& DeviceContext, FWindow& Window);
    ~FSwapchain();

    // --- 今天的核心任务：查询与决策 ---

    // 1. 查询显卡对 Surface 的支持情况
    static FSwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface);

    // 2. 选择最佳表面格式 (SRGB)
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::span<const VkSurfaceFormatKHR> AvailableFormats);

    // 3. 选择最佳呈现模式 (Mailbox > FIFO)
    static VkPresentModeKHR ChooseSwapPresentMode(std::span<const VkPresentModeKHR> AvailablePresentModes);

    // 4. 选择交换范围 (分辨率)
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, FWindow& Window);

private:
    FDeviceContext& DeviceContextRef;
    FWindow& WindowRef;

    // 可以在这里缓存一下今天的战果
    FSwapchainSupportDetails SupportDetails;
};