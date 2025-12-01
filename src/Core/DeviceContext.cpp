#include "Core/DeviceContext.h"
#include <stdexcept>
#include <iostream>
#include <map>
#include <set>
#include <optional>

namespace { // 匿名命名空间，相当于 C 语言的 static 全局变量，只在当前文件可见
#ifdef NDEBUG
    const bool bEnableValidationLayers = false;
#else
    const bool bEnableValidationLayers = true;
#endif

    const std::vector<const char*> ValidationLayers =
    {
        "VK_LAYER_KHRONOS_validation",
    };

    const std::vector<const char*> DeviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    // 1. 代理函数 (Proxy Functions)
    VkResult CreateDebugUtilsMessengerEXT(VkInstance Ininstance, const VkDebugUtilsMessengerCreateInfoEXT* InCreateInfo, const VkAllocationCallbacks* InAllocator, VkDebugUtilsMessengerEXT* InDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Ininstance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(Ininstance, InCreateInfo, InAllocator, InDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance InInstance, VkDebugUtilsMessengerEXT InDebugMessenger, const VkAllocationCallbacks* InAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(InInstance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(InInstance, InDebugMessenger, InAllocator);
        }
    }

    // 2. 调试回调函数
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT InMessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT InMessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* InCallbackData,
        void* pUserData)
    {

        if (InMessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            std::cerr << "validation layer: " << InCallbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& InCreateInfo)
    {
        InCreateInfo = {};
        InCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        InCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        InCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        InCreateInfo.pfnUserCallback = DebugCallback;
    }

    // 1. 队列族结构体定义
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        bool isComplete()
        {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }
    };

    // 2. 查找队列族
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice InDevice, VkSurfaceKHR InSurface)
    {
        QueueFamilyIndices Indices;

        uint32_t QueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(InDevice, &QueueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(InDevice, &QueueFamilyCount, QueueFamilies.data());

        int i = 0;
        for (const auto& QueueFamily : QueueFamilies)
        {
            // 检查是否支持图形指令
            if (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                Indices.GraphicsFamily = i;
            }

            // 检查是否支持显示到 Surface (Presentation)
            // [Senior Note]: 现在的驱动通常 Graphics 和 Present 是同一个队列，
            // 但早期硬件或特定架构可能是分开的，必须分别检查。
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(InDevice, i, InSurface, &presentSupport);
            if (presentSupport)
            {
                Indices.PresentFamily = i;
            }

            if (Indices.isComplete())
            {
                break;
            }
            i++;
        }

        return Indices;
    }

    // 3. 检查扩展
    bool CheckDeviceExtensionSupport(VkPhysicalDevice InDevice)
    {
        uint32_t ExtensionCount;
        vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &ExtensionCount, nullptr);

        std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
        vkEnumerateDeviceExtensionProperties(InDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

        std::set<std::string> RequiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

        for (const auto& Extension : AvailableExtensions)
        {
            RequiredExtensions.erase(Extension.extensionName);
        }

        return RequiredExtensions.empty();
    }

    int RateDeviceSuitability(VkPhysicalDevice InDevice, VkSurfaceKHR InSurface)
    {
        // 1. 属性 (名字、类型、版本)
        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(InDevice, &DeviceProperties);

        // 2. 特性 (几何着色器、细分曲面等)
        VkPhysicalDeviceFeatures DeviceFeatures;
        vkGetPhysicalDeviceFeatures(InDevice, &DeviceFeatures);

        int Score = 0;
        // A. 完整的队列族
        QueueFamilyIndices Indices = FindQueueFamilies(InDevice, InSurface);
        if (!Indices.isComplete()) return 0;

        // B. 所需的扩展 (Swapchain)
        if (!CheckDeviceExtensionSupport(InDevice)) return 0;

        // A. 各向异性过滤
        if (!DeviceFeatures.samplerAnisotropy) return 0;

        // B. 几何着色器
        if (!DeviceFeatures.geometryShader) return 0;

        // 1. 独显 (Discrete GPU) 
        if (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            Score += 1000;
        }

        // 2. 最大纹理尺寸
        Score += DeviceProperties.limits.maxImageDimension2D;

        std::cout << "Detected GPU: " << DeviceProperties.deviceName
            << " | Score: " << Score << std::endl;

        return Score;
    }
}

FDeviceContext::FDeviceContext(FWindow& WindowObj) : WindowRef(WindowObj)
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
}

FDeviceContext::~FDeviceContext()
{
    if (Surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(Instance, Surface, nullptr);
    }

    if (bEnableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(Instance, DebugMessenger, nullptr);
    }

    vkDestroyInstance(Instance, nullptr);
}

void FDeviceContext::CreateInstance()
{
    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "Vulkan Renderer";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName = "No Engine";
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

    auto Extensions = WindowRef.GetVulkanExtensions();
    Extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};
    if (bEnableValidationLayers)
    {
        Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = ValidationLayers.data();

        populateDebugMessengerCreateInfo(DebugCreateInfo);
        CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;
        CreateInfo.pNext = nullptr;
    }

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
    CreateInfo.ppEnabledExtensionNames = Extensions.data();

    std::cout << "Creating Vulkan Instance..." << std::endl;
    if (vkCreateInstance(&CreateInfo, nullptr, &Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
    std::cout << "Vulkan Instance created successfully!" << std::endl;
}

void FDeviceContext::SetupDebugMessenger()
{
    if (!bEnableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT CreateInfo{};
    // 复用填充逻辑
    populateDebugMessengerCreateInfo(CreateInfo);

    if (CreateDebugUtilsMessengerEXT(Instance, &CreateInfo, nullptr, &DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void FDeviceContext::CreateSurface()
{
    // SDL_Vulkan_CreateSurface 需要 SDL_Window* 指针
    if (!SDL_Vulkan_CreateSurface(WindowRef.GetNativeWindow(), Instance, &Surface))
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void FDeviceContext::PickPhysicalDevice()
{
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

    if (DeviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> Devices(DeviceCount);
    vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

    std::multimap<int, VkPhysicalDevice> Candidates;

    for (const auto& Device : Devices)
    {
        int Score = RateDeviceSuitability(Device, Surface);
        Candidates.insert(std::make_pair(Score, Device));
    }

    // rbegin() 是最后一个元素（分数最高的），且分数必须大于 0
    if (Candidates.rbegin()->first > 0)
    {
        PhysicalDevice = Candidates.rbegin()->second;

        // 打印最终选择
        VkPhysicalDeviceProperties Props;
        vkGetPhysicalDeviceProperties(PhysicalDevice, &Props);
        std::cout << ">>> SELECTED GPU: " << Props.deviceName << " <<<" << std::endl;
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void FDeviceContext::CreateLogicalDevice()
{
    QueueFamilyIndices Indices = FindQueueFamilies(PhysicalDevice, Surface);

    VkDeviceQueueCreateInfo QueueCreateInfo{};
    QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QueueCreateInfo.queueFamilyIndex = Indices.GraphicsFamily.value();
    QueueCreateInfo.queueCount = 1;
}
