#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "Swapchain.h"
#include "DeviceContext.h"
#include "DeviceSelector.h"

namespace { // 匿名命名空间，相当于 C 语言的 static 全局变量，只在当前文件可见
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
        Utils::ZeroVulkanStruct(InCreateInfo, VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
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

    CreateRenderPass();
}

FDeviceContext::~FDeviceContext()
{
    if (LogicalDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(LogicalDevice);
    }

    if (RenderPass)
    {
        vkDestroyRenderPass(LogicalDevice, RenderPass, nullptr);
    }

    if (Swapchain)
    {
        Swapchain.reset();
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
    Utils::ZeroVulkanStruct(AppInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
    AppInfo.pApplicationName = APP_NAME;
    AppInfo.applicationVersion = APP_VERSION;
    AppInfo.pEngineName = ENGINE_NAME;
    AppInfo.engineVersion = ENGINE_VERSION;
    AppInfo.apiVersion = VK_API_VERSION;

    VkInstanceCreateInfo CreateInfo{};
    Utils::ZeroVulkanStruct(CreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
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
        Utils::ZeroVulkanStruct(QueueCreateInfo, VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
        QueueCreateInfo.queueFamilyIndex = QueueFamily;
        QueueCreateInfo.queueCount = 1; // 每个族我们只需要 1 个队列句柄
        QueueCreateInfo.pQueuePriorities = &QueuePriority;

        QueueCreateInfos.push_back(QueueCreateInfo);
    }

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.geometryShader = VK_TRUE;

    VkDeviceCreateInfo CreateInfo{};
    Utils::ZeroVulkanStruct(CreateInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.pEnabledFeatures = &DeviceFeatures;

    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    Utils::ZeroVulkanStruct(vulkan12Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES);
    vulkan12Features.bufferDeviceAddress = VK_TRUE; // BDA
    vulkan12Features.descriptorIndexing = VK_TRUE; // Bindless

    CreateInfo.pNext = &vulkan12Features;
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

    AllocatorInfo.vulkanApiVersion = VK_API_VERSION;
    AllocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT; // Buffer Device Address (BDA)

    if (vmaCreateAllocator(&AllocatorInfo, &Allocator) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create VMA allocator!");
    }

    std::cout << "VMA Initialized Successfully." << std::endl;
}

void FDeviceContext::TestVMA()
{
    // 1. 定义 Buffer 信息 (创建一个 1KB 的顶点缓冲区)
    VkBufferCreateInfo bufferInfo {};
    Utils::ZeroVulkanStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferInfo.size = 1024; // 1KB
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO; // VMA 3.0+ 推荐的用法
    // 如果希望这块内存可以被 CPU 写入 (比如 Staging Buffer)，加上：
    // allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VkBuffer testBuffer;
    VmaAllocation testAlloc;

    check(Allocator != VK_NULL_HANDLE);
    VkResult result = vmaCreateBuffer(Allocator, &bufferInfo, &allocInfo, &testBuffer, &testAlloc, nullptr);

    if (result == VK_SUCCESS)
    {
        std::cout << "[VMA Test] Buffer created successfully via VMA!" << std::endl;
        vmaDestroyBuffer(Allocator, testBuffer, testAlloc);
    }
    else
    {
        throw std::runtime_error("[VMA Test] Failed to create buffer!");
    }
}

void FDeviceContext::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = Swapchain->GetImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo CreateInfo {};
    Utils::ZeroVulkanStruct(CreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    CreateInfo.attachmentCount = 1;
    CreateInfo.pAttachments = &colorAttachment;
    CreateInfo.dependencyCount = 1;
    CreateInfo.pDependencies = &dependency;
    CreateInfo.subpassCount = 1;
    CreateInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(LogicalDevice, &CreateInfo, nullptr, &RenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

VkShaderModule FDeviceContext::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    Utils::ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(LogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}
