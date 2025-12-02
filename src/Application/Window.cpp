#include "Window.h"
#include <stdexcept>
#include <vulkan/vulkan.h>

FWindow::FWindow(const std::string& InTitle, int InWidth, int InHeight)
    : Width(InWidth), Height(InHeight)
{

    // 设置窗口标志
    // SDL_WINDOW_VULKAN: 告诉 SDL 我们要用 Vulkan 渲染，不要创建 OpenGL 上下文
    // SDL_WINDOW_RESIZABLE: 允许用户拖拽改变窗口大小
    uint32_t Flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

    AppWindow = SDL_CreateWindow(
        InTitle.c_str(),          // 标题
        SDL_WINDOWPOS_CENTERED, // X 位置 (居中)
        SDL_WINDOWPOS_CENTERED, // Y 位置 (居中)
        InWidth,                  // 宽
        InHeight,                 // 高
        Flags                   // 标志位
    );

    if (!AppWindow)
    {
        throw std::runtime_error("Failed to create SDL Window: " + std::string(SDL_GetError()));
    }
}

FWindow::~FWindow()
{
    if (AppWindow)
    {
        SDL_DestroyWindow(AppWindow);
    }
}

std::vector<const char*> FWindow::GetVulkanExtensions() const
{
    unsigned int Count = 0;

    // 参数说明：窗口句柄, 数量指针, 名称数组(传nullptr表示只查数量)
    if (!SDL_Vulkan_GetInstanceExtensions(AppWindow, &Count, nullptr))
    {
        throw std::runtime_error("Failed to get Vulkan extensions count");
    }

    std::vector<const char*> Extensions(Count);

    // 3. 获取扩展名称
    if (!SDL_Vulkan_GetInstanceExtensions(AppWindow, &Count, Extensions.data()))
    {
        throw std::runtime_error("Failed to get Vulkan extensions names");
    }

    return Extensions;
}
