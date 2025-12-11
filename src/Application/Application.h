#pragma once
#include "Window.h"
#include "RHI/VulkanDevice.h"

class FApplication
{
public:
    FApplication();
    ~FApplication();

    void Init();

    void Run();

private:
    std::unique_ptr<FWindow> AppWindow;
    std::unique_ptr<FVulkanDevice> Context;

    bool bFramebufferResized = false;
};