#include "DeviceSelector.h"
#include <map>
#include <set>
#include <string>
#include <stdexcept>
#include <iostream>

namespace
{
    const std::vector<const char*> DeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}

FSelectionResult FDeviceSelector::Select(VkInstance InInstance, VkSurfaceKHR InSurface)
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(InInstance, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(InInstance, &DeviceCount, Devices.data());

    // 使用 map 自动排序
    std::multimap<int, VkPhysicalDevice> Candidates;

    for (const auto& Device : Devices)
    {
        int Score = RateDeviceSuitability(Device, InSurface);
        Candidates.insert(std::make_pair(Score, Device));
    }

    // 检查最佳结果
    if (Candidates.rbegin()->first > 0)
    {
        VkPhysicalDevice BestDevice = Candidates.rbegin()->second;

        FSelectionResult Result;
        Result.PhysicalDevice = BestDevice;
        Result.Score = Candidates.rbegin()->first;
        // 缓存 Indices，避免重复计算
        Result.Indices = FindQueueFamilies(BestDevice, InSurface);

        return Result;
    }

    throw std::runtime_error("failed to find a suitable GPU!");
}

// 查找队列族
FQueueFamilyIndices FDeviceSelector::FindQueueFamilies(VkPhysicalDevice InDevice, VkSurfaceKHR InSurface)
{
    FQueueFamilyIndices Indices;

    uint32_t QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(InDevice, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(InDevice, &QueueFamilyCount, QueueFamilies.data());

    int i = 0;
    for (int i = 0; i < QueueFamilies.size(); i++)
    {
        const auto& QueueFamily = QueueFamilies[i];

        if (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            Indices.GraphicsFamily = i;
        }

        VkBool32 bPresentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(InDevice, i, InSurface, &bPresentSupport);
        if (bPresentSupport)
        {
            Indices.PresentFamily = i;
        }
    }

    // 计算专用
    for (int i = 0; i < QueueFamilies.size(); i++)
    {
        const auto& QueueFamily = QueueFamilies[i];
        if ((QueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) &&
            !(QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            Indices.ComputeFamily = i;
            break;
        }
    }

    // 回退通用
    if (!Indices.ComputeFamily.has_value())
    {
        for (int i = 0; i < QueueFamilies.size(); i++)
        {
            const auto& QueueFamily = QueueFamilies[i];
            if (QueueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                Indices.ComputeFamily = i;
                break;
            }
        }
    }

    return Indices;
}

// 检查扩展
bool FDeviceSelector::CheckDeviceExtensionSupport(VkPhysicalDevice InDevice)
{
    uint32_t ExtensionCount;
    vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &ExtensionCount, nullptr);

    std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
    vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

    std::set<std::string> RequiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

    for (const auto& Extension : AvailableExtensions)
    {
        RequiredExtensions.erase(Extension.extensionName);
    }

    return RequiredExtensions.empty();
}

int FDeviceSelector::RateDeviceSuitability(VkPhysicalDevice InDevice, VkSurfaceKHR InSurface)
{
    // 1. 属性 (名字、类型、版本)
    VkPhysicalDeviceProperties DeviceProperties;
    vkGetPhysicalDeviceProperties(InDevice, &DeviceProperties);

    // 2. 特性 (几何着色器、细分曲面等)
    VkPhysicalDeviceFeatures DeviceFeatures;
    vkGetPhysicalDeviceFeatures(InDevice, &DeviceFeatures);

    int Score = 0;
    // A. 完整的队列族
    FQueueFamilyIndices Indices = FindQueueFamilies(InDevice, InSurface);
    if (!Indices.IsComplete()) return 0;

    // B. 所需的扩展 (Swapchain)
    if (!CheckDeviceExtensionSupport(InDevice)) return 0;

    // A. 各向异性过滤
    if (!DeviceFeatures.samplerAnisotropy) return 0;

    // B. 几何着色器
    if (!DeviceFeatures.geometryShader) return 0;

    // 1. 独显 (Discrete GPU) 
    if (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        Score += 1000;
    }

    // 2. 最大纹理尺寸
    Score += DeviceProperties.limits.maxImageDimension2D;

    std::cout << "Detected GPU: " << DeviceProperties.deviceName
        << " | Score: " << Score << std::endl;

    return Score;
}