#include "Application.h"
#include <SDL2/SDL.h>
#include <stdexcept> // 用于抛出异常
#include <iostream>

Application::Application() {
    // 1. 初始化 SDL 视频子系统
    // SDL_INIT_VIDEO 会自动初始化 Events 子系统
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        // SDL_GetError() 获取具体错误信息
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }

    // 2. 创建窗口 (Window 类的实例化)
    // 这里的 800, 600 是宽和高，"Vulkan Renderer" 是标题
    m_window = std::make_unique<Window>("Vulkan Renderer", 800, 600);

    // 3. 创建 Vulkan 上下文 (之后再实现)
    // m_context = std::make_unique<DeviceContext>(*m_window);
}

Application::~Application() {
    // 确保 Window 和 Context 先被销毁 (智能指针会自动处理，但逻辑上要注意)
    m_window.reset();
    // m_context.reset();

    // 4. 退出 SDL，清理所有子系统
    SDL_Quit();
    std::cout << "SDL Shutdown successfully." << std::endl;
}

void Application::run() {
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        // 处理 SDL 事件队列 (比如点击关闭按钮)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        // 这里以后会调用 m_context->draw();
    }
}