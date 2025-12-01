#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

struct FQueueFamilyIndices
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;
    std::optional<uint32_t> ComputeFamily;

    bool IsComplete() const
    {
        return GraphicsFamily.has_value() && PresentFamily.has_value();
    }
};

struct FSelectionResult
{
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    FQueueFamilyIndices Indices;
    int Score = 0;
};

class FDeviceSelector
{
public:
    static FSelectionResult Select(VkInstance Instance, VkSurfaceKHR Surface);

private:
    static FQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device, VkSurfaceKHR Surface);
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice Device);
    static int RateDeviceSuitability(VkPhysicalDevice Device, VkSurfaceKHR Surface);
};