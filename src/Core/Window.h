#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <string>
#include <vector>

class Window
{
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    // 禁止拷贝，防止窗口句柄被意外复制
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // 获取 SDL 窗口原始指针（传给 Vulkan 用）
    SDL_Window* getNativeWindow() const { return m_window; }

    // 获取窗口扩展（Vulkan 创建 Instance 需要知道它要在这个窗口上画画）
    std::vector<const char*> getVulkanExtensions() const;

private:
    SDL_Window* m_window = nullptr;
    int m_width;
    int m_height;
};