#include "Core/Window.h"
#include <stdexcept>
#include <vulkan/vulkan.h>

Window::Window(const std::string& title, int width, int height)
    : m_width(width), m_height(height) {

    // 设置窗口标志
    // SDL_WINDOW_VULKAN: 告诉 SDL 我们要用 Vulkan 渲染，不要创建 OpenGL 上下文
    // SDL_WINDOW_RESIZABLE: 允许用户拖拽改变窗口大小
    uint32_t flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

    m_window = SDL_CreateWindow(
        title.c_str(),          // 标题
        SDL_WINDOWPOS_CENTERED, // X 位置 (居中)
        SDL_WINDOWPOS_CENTERED, // Y 位置 (居中)
        width,                  // 宽
        height,                 // 高
        flags                   // 标志位
    );

    if (!m_window) {
        throw std::runtime_error("Failed to create SDL Window: " + std::string(SDL_GetError()));
    }
}

Window::~Window() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
}

std::vector<const char*> Window::getVulkanExtensions() const
{
    unsigned int count = 0;

    // 参数说明：窗口句柄, 数量指针, 名称数组(传nullptr表示只查数量)
    if (!SDL_Vulkan_GetInstanceExtensions(m_window, &count, nullptr)) {
        throw std::runtime_error("Failed to get Vulkan extensions count");
    }

    std::vector<const char*> extensions(count);

    // 3. 获取扩展名称
    if (!SDL_Vulkan_GetInstanceExtensions(m_window, &count, extensions.data())) {
        throw std::runtime_error("Failed to get Vulkan extensions names");
    }

    return extensions;
}
