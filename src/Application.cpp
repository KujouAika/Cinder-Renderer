#include "Application.h"
#include <SDL2/SDL.h>
#include <stdexcept> // 用于抛出异常
#include <iostream>

FApplication::FApplication()
{
    // 1. 初始化 SDL 视频子系统
    // SDL_INIT_VIDEO 会自动初始化 Events 子系统
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }

    AppWindow = std::make_unique<FWindow>("Vulkan Renderer", 800, 600);
    Context = std::make_unique<FDeviceContext>(*AppWindow);
}

FApplication::~FApplication()
{
    // 确保 Window 和 Context 先被销毁 (智能指针会自动处理，但逻辑上要注意)
    AppWindow.reset();
    Context.reset();

    SDL_Quit();
    std::cout << "SDL Shutdown successfully." << std::endl;
}

void FApplication::Run()
{
    bool bIsRunning = true;
    SDL_Event event;

    while (bIsRunning)
    {
        // 处理 SDL 事件队列 (比如点击关闭按钮)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                bIsRunning = false;
            }
        }
    }
}