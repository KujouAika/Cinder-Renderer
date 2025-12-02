#include "Swapchain.h"
#include "DeviceContext.h"
#include "Application/Window.h"
#include <algorithm>
#include <iostream>

// 构造函数暂时留空，或者先做一次查询测试
FSwapchain::FSwapchain(FDeviceContext& InDeviceContext, FWindow& InWindow)
    : DeviceContextRef(InDeviceContext), WindowRef(InWindow)
{
    SupportDetails = QuerySwapChainSupport(DeviceContextRef.GetPhysicalDevice(), InDeviceContext.GetSurface());
}

FSwapchain::~FSwapchain()
{
    // 暂时没东西销毁
}

FSwapchainSupportDetails FSwapchain::QuerySwapChainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR Surface)
{
    FSwapchainSupportDetails Details;

    // 1. 获取基础能力 (Capabilities)
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &Details.Capabilities);

    // 2. 获取格式 (Formats)
    uint32_t FormatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, nullptr);

    if (FormatCount != 0)
    {
        Details.Formats.resize(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, Details.Formats.data());
    }

    // 3. 获取呈现模式 (Present Modes)
    uint32_t PresentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, nullptr);

    if (PresentModeCount != 0)
    {
        Details.PresentModes.resize(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &PresentModeCount, Details.PresentModes.data());
    }

    return Details;
}

VkSurfaceFormatKHR FSwapchain::ChooseSwapSurfaceFormat(std::span<const VkSurfaceFormatKHR> AvailableFormats)
{
    // 优先寻找：B8G8R8A8_SRGB + SRGB_NONLINEAR
    // SRGB 格式能让 GPU 自动进行线性空间到 SRGB 的 Gamma 校正，这是 PBR 渲染的基础。
    for (const auto& AvailableFormat : AvailableFormats)
    {
        if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            std::cout << "Swapchain Format: VK_FORMAT_B8G8R8A8_SRGB (Best)" << std::endl;
            return AvailableFormat;
        }
    }

    // 如果没找到，退而求其次，返回列表里的第一个
    // (实际项目中可能还可以找 UNORM 格式手动校正，但通常 SRGB 都是支持的)
    std::cout << "Swapchain Format: Fallback (First Available)" << std::endl;
    return AvailableFormats[0];
}

VkPresentModeKHR FSwapchain::ChooseSwapPresentMode(std::span<const VkPresentModeKHR> AvailablePresentModes)
{
    // [Senior Strategy]
    // 1. Mailbox (三缓冲): 显卡一直画，把最新的一帧给屏幕。延迟低，无撕裂。
    for (const auto& AvailablePresentMode : AvailablePresentModes)
    {
        if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            std::cout << "Present Mode: Mailbox (Best)" << std::endl;
            return AvailablePresentMode;
        }
    }

    // 2. FIFO (垂直同步): 队列满了就等。省电，无撕裂，但有延迟。
    // Vulkan 规范保证 FIFO 一定被支持，所以它是完美的保底方案。
    std::cout << "Present Mode: FIFO (V-Sync)" << std::endl;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D FSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities, FWindow& Window)
{
    // 如果 currentExtent.width 不是最大值，说明窗口系统已经定死了分辨率（通常如此）
    if (Capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return Capabilities.currentExtent;
    }
    else
    {
        // 如果是最大值，说明允许我们自己定（比如高 DPI 屏幕）
        // 这里需要从 SDL 获取实际像素大小 (Drawable Size)
        int Width, Height;
        SDL_Vulkan_GetDrawableSize(Window.GetNativeWindow(), &Width, &Height);

        VkExtent2D ActualExtent = {
            static_cast<uint32_t>(Width),
            static_cast<uint32_t>(Height)
        };

        // 夹紧在 min 和 max 之间
        ActualExtent.width = std::clamp(ActualExtent.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        ActualExtent.height = std::clamp(ActualExtent.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

        return ActualExtent;
    }
}