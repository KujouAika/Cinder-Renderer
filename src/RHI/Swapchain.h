#pragma once
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

    void Create();
    void Cleanup();

    VkSwapchainKHR GetHandle() const { return Swapchain; }
    VkFormat GetImageFormat() const { return ImageFormat; }
    VkExtent2D GetExtent() const { return Extent; }

    const std::vector<VkImage>& GetImages() const { return Images; }

    static FSwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface);
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::span<const VkSurfaceFormatKHR> AvailableFormats);
    static VkPresentModeKHR ChooseSwapPresentMode(std::span<const VkPresentModeKHR> AvailablePresentModes);
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, FWindow& Window);

private:
    FDeviceContext& DeviceContextRef;
    FWindow& WindowRef;

    VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> Images;
    VkFormat ImageFormat;
    VkExtent2D Extent;

    FSwapchainSupportDetails SupportDetails;
};