#pragma once

class FRHISwapchain {
public:
    virtual ~FRHISwapchain() = default;

    virtual void Resize(const uint32_t& Width, const uint32_t& Height) = 0;

    virtual bool GetNextImage(uint32_t& OutImageIndex, void* SignalSemaphore) = 0;

    virtual bool Present(uint32_t ImageIndex, void* InWaitSemaphore) = 0;

    virtual RHIFormat GetFormat() const = 0;
    virtual RHIExtent2D GetExtent() const = 0;
    virtual uint32_t GetImageCount() const = 0;
};