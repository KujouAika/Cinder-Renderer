#include "Swapchain.h"
#include "DeviceContext.h"
#include "Application/Window.h"

FSwapchain::FSwapchain(FDeviceContext& InDeviceContext, FWindow& InWindow)
    : DeviceContextRef(InDeviceContext), WindowRef(InWindow)
{
    Create();
}

FSwapchain::~FSwapchain()
{
    Cleanup();
}

void FSwapchain::Create()
{
    SupportDetails = QuerySwapChainSupport(DeviceContextRef.GetPhysicalDevice(), DeviceContextRef.GetSurface());
    VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SupportDetails.Formats);
    VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SupportDetails.PresentModes);
    VkExtent2D SwapchainExtent = ChooseSwapExtent(SupportDetails.Capabilities, WindowRef);

    // + 1 防止驱动程序在等待 VSync 时阻塞 CPU
    uint32_t ImageCount = SupportDetails.Capabilities.minImageCount + 1;

    if (SupportDetails.Capabilities.maxImageCount > 0 && ImageCount > SupportDetails.Capabilities.maxImageCount)
    {
        ImageCount = SupportDetails.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = DeviceContextRef.GetSurface();

    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = SurfaceFormat.format;
    CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    CreateInfo.imageExtent = SwapchainExtent;
    CreateInfo.imageArrayLayers = 1; // 非 VR 始终为 1
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    CreateInfo.imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;

    FQueueFamilyIndices Indices = DeviceContextRef.GetQueueFamilyIndices();
    uint32_t QueueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

    if (Indices.GraphicsFamily != Indices.PresentFamily)
    {
        // 如果图形和呈现不是同一个队列 (罕见，但存在)，需要并发模式
        // 这样就不需要手动转移所有权了 (虽然性能略低，但安全)
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
    }
    else
    {
        // 最常见情况：独占模式 (性能最佳)
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CreateInfo.queueFamilyIndexCount = 0; // Optional
        CreateInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    CreateInfo.preTransform = SupportDetails.Capabilities.currentTransform; // 不做旋转 (移动端可能需要处理)
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // 不透明，忽略窗口系统的 Alpha 通道
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE; // 被其他窗口挡住的像素不予计算 (Clip)

    // Resize
    VkSwapchainKHR OldSwapchain = Swapchain;
    VkSwapchainKHR NewSwapchain = VK_NULL_HANDLE;
    if (Swapchain != VK_NULL_HANDLE)
    {
        CreateInfo.oldSwapchain = Swapchain;
    }
    
    if (vkCreateSwapchainKHR(DeviceContextRef.GetLogicalDevice(), &CreateInfo, nullptr, &NewSwapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }
    Swapchain = NewSwapchain;
    if (OldSwapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(DeviceContextRef.GetLogicalDevice(), OldSwapchain, nullptr);
    }
    
    ImageFormat = SurfaceFormat.format;
    Extent = SwapchainExtent;

    vkGetSwapchainImagesKHR(DeviceContextRef.GetLogicalDevice(), Swapchain, &ImageCount, nullptr);
    Images.resize(ImageCount);
    vkGetSwapchainImagesKHR(DeviceContextRef.GetLogicalDevice(), Swapchain, &ImageCount, Images.data());

    std::cout << "Swapchain created successfully!" << std::endl;
    std::cout << "  - Format: " << ImageFormat << std::endl;
    std::cout << "  - Extent: " << Extent.width << "x" << Extent.height << std::endl;
    std::cout << "  - Image Count: " << Images.size() << std::endl;
}

void FSwapchain::Cleanup()
{
    ImageViews.clear();
    
    if (Swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(DeviceContextRef.GetLogicalDevice(), Swapchain, nullptr);
        Swapchain = VK_NULL_HANDLE;
    }
}

void FSwapchain::Recreate()
{
    Create();
    CreateImageViews();

    for (size_t i = 0; i < ImageViews.size(); i++)
    {
        check(ImageViews[i].Get() != VK_NULL_HANDLE);
    }
    std::cout << "[Test] All " << ImageViews.size() << " ImageViews created successfully." << std::endl;
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

void FSwapchain::CreateImageViews()
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = ImageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    
    // Deleter
    TVulkanHandle<VkImageView>::FDeleter ImageViewDeleter =
        [](VkDevice InDevice, VkImageView InImageView, const VkAllocationCallbacks* pAllocator)
        {
            vkDestroyImageView(InDevice, InImageView, pAllocator);
        };

    ImageViews.clear();

    for (size_t i = 0; i < Images.size(); i++)
    {
        createInfo.image = Images[i];
        VkImageView imageViewHandle = VK_NULL_HANDLE;
        if (vkCreateImageView(DeviceContextRef.GetLogicalDevice(), &createInfo, nullptr, &imageViewHandle) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
        ImageViews.emplace_back(DeviceContextRef.GetLogicalDevice(), imageViewHandle, ImageViewDeleter);
    }
}
