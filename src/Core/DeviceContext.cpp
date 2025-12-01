#include "Core/DeviceContext.h"
#include "Core/DeviceSelector.h"
#include <stdexcept>
#include <iostream>
#include <set>


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
    FSelectionResult Result = FDeviceSelector::Select(Instance, Surface);
    PhysicalDevice = Result.PhysicalDevice;
    QueueIndices = Result.Indices;

    VkPhysicalDeviceProperties Props;
    vkGetPhysicalDeviceProperties(PhysicalDevice, &Props);
    std::cout << "Selected GPU: " << Props.deviceName << std::endl;
}

void FDeviceContext::CreateLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    std::set<uint32_t> UniqueQueueFamilies =
    {
        QueueIndices.GraphicsFamily.value(),
        QueueIndices.PresentFamily.value()
    };
}
