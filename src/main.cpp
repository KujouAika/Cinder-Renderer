#include <iostream>
#include <vector>

// 包含 SDL 头文件
// 注意：因为你在 CMake 中将 include 路径设为了 .../Include/SDL2
// 所以直接 include <SDL.h> 即可，不需要写 <SDL2/SDL.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <glm/glm.hpp>
// 包含 Vulkan 头文件
#include <vulkan/vulkan.h>

int main(int argc, char* argv[]) {
    // 1. 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "1. SDL Initialized successfully." << std::endl;

    // 2. 创建窗口 (必须加上 SDL_WINDOW_VULKAN 标志)
    SDL_Window* window = SDL_CreateWindow(
        "Vulkan Test Window",       // 窗口标题
        SDL_WINDOWPOS_CENTERED,     // x
        SDL_WINDOWPOS_CENTERED,     // y
        800, 600,                   // 宽, 高
        SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    std::cout << "2. Vulkan Window Created successfully." << std::endl;

    // 3. 获取创建 Vulkan 实例所需的 SDL 扩展
    unsigned int extensionCount = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr)) {
        std::cerr << "Failed to get Vulkan extensions count." << std::endl;
        return 1;
    }

    std::vector<const char*> extensions(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data())) {
        std::cerr << "Failed to get Vulkan extensions names." << std::endl;
        return 1;
    }

    std::cout << "   Required Extensions count: " << extensionCount << std::endl;

    // 4. 创建 Vulkan 实例 (这是验证 Vulkan 链接是否成功的关键)
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Vulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0; // 暂时不开启校验层

    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan Instance! Error code: " << result << std::endl;
        return 1;
    }
    std::cout << "3. Vulkan Instance Created successfully!" << std::endl;

    // --- 简单的事件循环，让窗口停留一会 ---
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    // 5. 清理资源
    vkDestroyInstance(instance, nullptr); // 销毁 Vulkan 实例
    SDL_DestroyWindow(window);            // 销毁窗口
    SDL_Quit();                           // 退出 SDL

    std::cout << "Cleaned up and exiting." << std::endl;
    return 0;
}