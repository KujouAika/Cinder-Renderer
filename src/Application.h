#pragma once
#include "Core/Window.h"
#include "Core/DeviceContext.h"
#include <memory>

class Application
{
public:
    Application();
    ~Application();

    void run(); // 主循环：处理事件 -> 渲染

private:
    // 使用智能指针管理生命周期，或者直接栈对象也可以，看个人喜好
    // 注意初始化顺序：Window 必须在 DeviceContext 之前！
    std::unique_ptr<Window> m_window;
    std::unique_ptr<DeviceContext> m_context;
};