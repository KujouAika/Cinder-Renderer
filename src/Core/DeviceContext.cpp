#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "Core/Swapchain.h"
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

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& InCreateInfo)
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

    CreateAllocator();

    Swapchain = std::make_unique<FSwapchain>(*this, WindowRef);
}

FDeviceContext::~FDeviceContext()
{
    if (LogicalDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(LogicalDevice);
    }

    if (Allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(Allocator);
        Allocator = VK_NULL_HANDLE;
    }

    if (LogicalDevice != VK_NULL_HANDLE)
    {
        vkDestroyDevice(LogicalDevice, nullptr);
        LogicalDevice = VK_NULL_HANDLE;
    }

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

        PopulateDebugMessengerCreateInfo(DebugCreateInfo);
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
    PopulateDebugMessengerCreateInfo(CreateInfo);

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

    std::cout << "Graphics Family Index: " << QueueIndices.GraphicsFamily.value() << std::endl;
    std::cout << "Present Family Index: " << QueueIndices.PresentFamily.value() << std::endl;
    std::cout << "Compute Family Index:  " << QueueIndices.ComputeFamily.value() << std::endl;

    if (QueueIndices.GraphicsFamily.value() != QueueIndices.ComputeFamily.value())
    {
        std::cout << ">> Dedicated Async Compute Queue Found! (True Async)" << std::endl;
    }
    else
    {
        std::cout << ">> Shared Graphics/Compute Queue. (Fallback)" << std::endl;
    }
}

void FDeviceContext::CreateLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    float QueuePriority = 1.0f;
    std::set<uint32_t> UniqueQueueFamilies =
    {
        QueueIndices.GraphicsFamily.value(),
        QueueIndices.PresentFamily.value(),
        QueueIndices.ComputeFamily.value()
    };

    for (uint32_t QueueFamily : UniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = QueueFamily;
        QueueCreateInfo.queueCount = 1; // 每个族我们只需要 1 个队列句柄
        QueueCreateInfo.pQueuePriorities = &QueuePriority;

        QueueCreateInfos.push_back(QueueCreateInfo);
    }

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.geometryShader = VK_TRUE;

    VkDeviceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.pEnabledFeatures = &DeviceFeatures;

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    if (bEnableValidationLayers)
    {
        CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }
    else
    {
        CreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(PhysicalDevice, &CreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(LogicalDevice, QueueIndices.GraphicsFamily.value(), 0, &GraphicsQueue);
    vkGetDeviceQueue(LogicalDevice, QueueIndices.PresentFamily.value(), 0, &PresentQueue);
    vkGetDeviceQueue(LogicalDevice, QueueIndices.ComputeFamily.value(), 0, &ComputeQueue);
}

void FDeviceContext::CreateAllocator()
{
    VmaAllocatorCreateInfo AllocatorInfo{};

    AllocatorInfo.physicalDevice = PhysicalDevice;
    AllocatorInfo.device = LogicalDevice;
    AllocatorInfo.instance = Instance;

    AllocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    AllocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT; // Buffer Device Address (BDA)

    if (vmaCreateAllocator(&AllocatorInfo, &Allocator) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create VMA allocator!");
    }

    std::cout << "VMA Initialized Successfully." << std::endl;
}