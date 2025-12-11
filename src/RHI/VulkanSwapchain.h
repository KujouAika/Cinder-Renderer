#pragma once
#include "RHI/RHISwapchain.h"
// 前置声明
class FVulkanDevice;
class FWindow;

// 辅助结构体：一次性打包所有查询到的支持信息
struct FSwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities;      // 基础能力（宽高限制、图片数量限制）
    std::vector<VkSurfaceFormatKHR> Formats;    // 支持的像素格式（SRGB, Linear...）
    std::vector<VkPresentModeKHR> PresentModes; // 支持的呈现模式（FIFO, Mailbox...）
};

class FVulkanSwapchain: public FRHISwapchain
{
public:
    FVulkanSwapchain(const uint32_t& InWidth, const uint32_t& InHeight, FVulkanDevice& InDeviceRef, FWindow& InWindowRef);
    virtual ~FVulkanSwapchain();

    virtual void Resize(const uint32_t& Width, const uint32_t& Height) override;
    virtual bool GetNextImage(uint32_t& OutImageIndex, void* InSignalSemaphore) override;
    virtual bool Present(uint32_t ImageIndex, void* InWaitSemaphore) override;

    void Create(const uint32_t& Width, const uint32_t& Height);
    void Cleanup();

    virtual RHIFormat GetFormat() const override
    {
        return RHIFormat::R8G8B8A8_UNORM; // TODO: 需匹配 SwapchainImageFormat
    }
    virtual uint32_t GetImageCount() const override
    {
        return static_cast<uint32_t>(Images.size());
    }
    virtual RHIExtent2D GetExtent() const override { return { Extent.width, Extent.height }; }

    VkSwapchainKHR GetHandle() const { check(Swapchain != VK_NULL_HANDLE); return Swapchain; }
    VkFormat GetVkFormat() const { return ImageFormat; }
    VkExtent2D GetVkExtent() const { return Extent; }

    const std::vector<VkImage>& GetImages() const { return Images; }
    const std::vector<TVulkanHandle<VkImageView>>& GetImageViews() const { return ImageViews; }

    static FSwapchainSupportDetails QuerySwapChainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface);
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::span<const VkSurfaceFormatKHR> AvailableFormats);
    static VkPresentModeKHR ChooseSwapPresentMode(std::span<const VkPresentModeKHR> AvailablePresentModes);
    static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, FWindow& Window, const int& Width, const int& Height);

private:
    FVulkanDevice& DeviceRef;
    FWindow& WindowRef;

    VkDevice LogicalDevice;
    VkPhysicalDevice PhysicalDevice;
    VkSurfaceKHR Surface;

    VkFormat ImageFormat;
    VkExtent2D Extent;
    VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> Images;
    std::vector<TVulkanHandle<VkImageView>> ImageViews;
};