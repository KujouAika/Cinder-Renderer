#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <string>
#include <vector>

class FWindow
{
public:
    FWindow(const std::string& title, int width, int height);
    ~FWindow();

    // 禁止拷贝，防止窗口句柄被意外复制
    FWindow(const FWindow&) = delete;
    FWindow& operator=(const FWindow&) = delete;

    // 获取 SDL 窗口原始指针（传给 Vulkan 用）
    SDL_Window* GetNativeWindow() const { return AppWindow; }

    // 获取窗口扩展（Vulkan 创建 Instance 需要知道它要在这个窗口上画画）
    std::vector<const char*> GetVulkanExtensions() const;

private:
    SDL_Window* AppWindow = nullptr;
    int Width;
    int Height;
};