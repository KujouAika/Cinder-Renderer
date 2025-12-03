#include <vulkan/vulkan.h>
#include <functional>

template<typename T>
class TVulkanHandle
{
public:
    using FDeleter = std::function<void(VkDevice, T, const VkAllocationCallbacks*)>;
    
    T Get() const { return Handle; }

    TVulkanHandle(VkDevice InDevice, T InHandle, FDeleter InDeleter)
        : Device(InDevice), Handle(InHandle), Deleter(InDeleter)
    {
    }

    ~TVulkanHandle() 
    {
        if (Handle != VK_NULL_HANDLE && Deleter)
        {
            Deleter(Device, Handle, nullptr);
        }
    }

    TVulkanHandle(const TVulkanHandle&) = delete;
    TVulkanHandle& operator=(const TVulkanHandle&) = delete;

    TVulkanHandle(TVulkanHandle&& Other) noexcept
        : Handle(Other.Handle), Device(Other.Device), Deleter(std::move(Other.Deleter))
    {
        Other.Handle = VK_NULL_HANDLE;
    }

    TVulkanHandle& operator=(TVulkanHandle&& Other) noexcept
        {
        if (this != &Other)
        {
            // 释放当前资源
            if (Handle != VK_NULL_HANDLE && Deleter)
            {
                Deleter(Device, Handle, nullptr);
            }
            // 转移资源所有权
            Handle = Other.Handle;
            Device = Other.Device;
            Deleter = std::move(Other.Deleter);
            Other.Handle = VK_NULL_HANDLE;
        }
        return *this;
    }

    operator T() const { return Handle; }

    explicit operator bool() const { return Handle != VK_NULL_HANDLE; }

private:
    T Handle = VK_NULL_HANDLE;
    VkDevice Device = VK_NULL_HANDLE;
    FDeleter Deleter;
};