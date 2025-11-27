#include "Core/DeviceContext.h"
#include <stdexcept>
#include <iostream>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

DeviceContext::DeviceContext(Window& window) : m_windowRef(window) {
    createInstance();
}

DeviceContext::~DeviceContext() {
    vkDestroyInstance(m_instance, nullptr);
}

void DeviceContext::createInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = m_windowRef.getVulkanExtensions();

    // [Senior Tip]: 在 Debug 模式下，通常还需要 VK_EXT_debug_utils 扩展
    // 这里暂时先只加 SDL 需要的，保证能跑
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // --- 4. 设置验证层 (Layers) ---
    // [Senior Tip]: 实际工程中这里应该用 #ifdef NDEBUG 宏来控制，Release 包不开启验证层
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    std::cout << "Creating Vulkan Instance..." << std::endl;
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
    std::cout << "Vulkan Instance created successfully!" << std::endl;
}

void DeviceContext::setupDebugMessenger()
{
}

void DeviceContext::pickPhysicalDevice()
{
}

void DeviceContext::createLogicalDevice()
{
}

int DeviceContext::rateDeviceSuitability(VkPhysicalDevice device)
{
    return 0;
}
