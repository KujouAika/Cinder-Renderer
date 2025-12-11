#pragma once
#include "Window.h"
#include "RHI/VulkanDevice.h"

class FApplication
{
public:
    FApplication();
    ~FApplication();

    void Init();

    void Run(); // 主循环：处理事件 -> 渲染

private:
    // 使用智能指针管理生命周期，或者直接栈对象也可以，看个人喜好
    // 注意初始化顺序：Window 必须在 DeviceContext 之前！
    std::unique_ptr<FWindow> AppWindow;
    std::unique_ptr<FVulkanDevice> Context;

    bool bFramebufferResized = false;
};