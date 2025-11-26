#include "Core/DeviceContext.h"

DeviceContext::DeviceContext(Window& window) : m_windowRef(window) {
    createInstance();
}

// !!! 你可能缺了这个 !!!
// 析构函数：即使现在没东西要销毁，也要写个空函数体
DeviceContext::~DeviceContext() {
    // 后面我们要在这里调用 vkDestroyInstance(m_instance, nullptr);
}

void DeviceContext::createInstance()
{
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
