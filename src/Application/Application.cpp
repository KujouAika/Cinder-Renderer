#include "Application.h"
#include <SDL2/SDL.h>
#include <thread>
#include <chrono>

FApplication::FApplication()
{
    // 1. 初始化 SDL SDL_INIT_VIDEO 会自动初始化 Events 子系统
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }
}

FApplication::~FApplication()
{
    // 确保 Window 和 Context 先被销毁 (智能指针会自动处理，但逻辑上要注意)
    AppWindow.reset();
    Context.reset();

    SDL_Quit();
    std::cout << "SDL Shutdown successfully." << std::endl;
}

void FApplication::Init()
{
    AppWindow = std::make_unique<FWindow>("Vulkan Renderer", 800, 600);
    Context = std::make_unique<FDeviceContext>(*AppWindow);
    Context->Init();
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
            else if (event.type == SDL_WINDOWEVENT)
            {
                // SDL_WINDOWEVENT_RESIZED: 用户拖拽调整大小
                // SDL_WINDOWEVENT_SIZE_CHANGED: 窗口系统改变大小
                if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    bFramebufferResized = true;
                }
            }
        }
        
        if (bFramebufferResized)
        {
            int width = 0, height = 0;
            AppWindow->GetSize(width, height);

            // 最小化暂停渲染等待窗口恢复。
            while (width == 0 || height == 0)
            {
                SDL_WaitEvent(&event); // 阻塞等待新事件，降低 CPU 占用
                if (event.type == SDL_QUIT)
                {
                    bIsRunning = false;
                    break;
                }
                AppWindow->GetSize(width, height);
            }
            std::cout << "[Test] Triggering Swapchain Recreation..." << std::endl;
            Context->RecreateSwapchain();
            bFramebufferResized = false;
            std::cout << "[Test] Swapchain Recreated." << std::endl;

        }

        if (!bIsRunning) break;

        try
        {
            // std::this_thread::sleep_for(std::chrono::milliseconds(16));
            bool bRenderSuccess = Context->RenderFrame();

            if (!bRenderSuccess) {
                std::cout << "[Vulkan] Driver requested swapchain recreation." << std::endl;
                // 将标志位置为 true，下一次 while 循环开头就会触发重建逻辑
                bFramebufferResized = true;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Render Error: " << e.what() << std::endl;
        }
    }
    vkDeviceWaitIdle(Context->GetLogicalDevice());
}