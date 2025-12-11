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

FDeviceContext::FDeviceContext(FWindow& WindowObj) : WindowRef(WindowObj) {}

void FDeviceContext::Init()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();

    CreateAllocator();

    Swapchain = std::make_unique<FSwapchain>(*this, WindowRef);

    CreatePipelineLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateCommandBuffers();

    CreateSyncObjects();
}

void FDeviceContext::RecreateSwapchain()
{
    vkDeviceWaitIdle(LogicalDevice);
    Swapchain->Recreate();
}

FDeviceContext::~FDeviceContext()
{
    if (LogicalDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(LogicalDevice);
    }

    if (GraphicsTimelineSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(LogicalDevice, GraphicsTimelineSemaphore, nullptr);
    }

    for (VkSemaphore Semaphore : PresentSemaphores)
    {
        vkDestroySemaphore(LogicalDevice, Semaphore, nullptr);
    }

    for (VkSemaphore Semaphore : ImageAvailableSemaphores)
    {
        vkDestroySemaphore(LogicalDevice, Semaphore, nullptr);
    }

    if (CommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(LogicalDevice, CommandPool, nullptr);
    }

    if (GraphicsPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(LogicalDevice, GraphicsPipeline, nullptr);
    }

    if (PipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, nullptr);
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

    VkDeviceCreateInfo CreateInfo{};
    Utils::ZeroVulkanStruct(CreateInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
    CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
    CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
    CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    Utils::ZeroVulkanStruct(vulkan12Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES);
    vulkan12Features.bufferDeviceAddress = VK_TRUE; // BDA
    vulkan12Features.descriptorIndexing = VK_TRUE; // Bindless
    vulkan12Features.timelineSemaphore = VK_TRUE; // Timeline Semaphore

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
    Utils::ZeroVulkanStruct(dynamicRenderingFeature, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES);
    dynamicRenderingFeature.dynamicRendering = VK_TRUE;
    vulkan12Features.pNext = &dynamicRenderingFeature;

    VkPhysicalDeviceSynchronization2Features synchronization2Features{};
    Utils::ZeroVulkanStruct(synchronization2Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES);
    synchronization2Features.synchronization2 = VK_TRUE;
    dynamicRenderingFeature.pNext = &synchronization2Features;

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.geometryShader = VK_TRUE;
    
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    Utils::ZeroVulkanStruct(physicalDeviceFeatures2, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
    physicalDeviceFeatures2.features = DeviceFeatures;
    physicalDeviceFeatures2.pNext = &vulkan12Features;

    CreateInfo.pNext = &physicalDeviceFeatures2;

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

//void FDeviceContext::CreateRenderPass()
//{
//    VkAttachmentDescription colorAttachment{};
//    colorAttachment.format = Swapchain->GetImageFormat();
//    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//    VkSubpassDependency dependency{};
//    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//    dependency.dstSubpass = 0;
//    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//    dependency.srcAccessMask = 0;
//    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
//
//    VkAttachmentReference colorAttachmentRef{};
//    colorAttachmentRef.attachment = 0;
//    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//    VkSubpassDescription subpass{};
//    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    subpass.colorAttachmentCount = 1;
//    subpass.pColorAttachments = &colorAttachmentRef;
//
//    VkRenderPassCreateInfo CreateInfo {};
//    Utils::ZeroVulkanStruct(CreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
//    CreateInfo.attachmentCount = 1;
//    CreateInfo.pAttachments = &colorAttachment;
//    CreateInfo.dependencyCount = 1;
//    CreateInfo.pDependencies = &dependency;
//    CreateInfo.subpassCount = 1;
//    CreateInfo.pSubpasses = &subpass;
//
//    if (vkCreateRenderPass(LogicalDevice, &CreateInfo, nullptr, &RenderPass) != VK_SUCCESS)
//    {
//        throw std::runtime_error("failed to create render pass!");
//    }
//}

VkShaderModule FDeviceContext::CreateShaderModule(const std::vector<char>& InCode) const
{
    VkShaderModuleCreateInfo CreateInfo{};
    Utils::ZeroVulkanStruct(CreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
    CreateInfo.codeSize = InCode.size();
    CreateInfo.pCode = reinterpret_cast<const uint32_t*>(InCode.data());

    VkShaderModule ShaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(LogicalDevice, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create shader module!");
    }
    return ShaderModule;
}

void FDeviceContext::CreatePipelineLayout()
{
    VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
    Utils::ZeroVulkanStruct(PipelineLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
    PipelineLayoutInfo.setLayoutCount = 0;
    PipelineLayoutInfo.pSetLayouts = nullptr;
    PipelineLayoutInfo.pushConstantRangeCount = 0;
    PipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(LogicalDevice, &PipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void FDeviceContext::CreateGraphicsPipeline()
{
    VkPipelineShaderStageCreateInfo VertexShaderStageInfo{};
    Utils::ZeroVulkanStruct(VertexShaderStageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    VertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VkShaderModule vertexShaderModule = CreateShaderModule(Utils::ReadSPV("Triangle.vert.spv"));
    VertexShaderStageInfo.module = vertexShaderModule;
    VertexShaderStageInfo.pName = "VSMain";

    VkPipelineShaderStageCreateInfo FragmentShaderStageInfo{};
    Utils::ZeroVulkanStruct(FragmentShaderStageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    FragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkShaderModule fragmentShaderModule = CreateShaderModule(Utils::ReadSPV("Triangle.frag.spv"));
    FragmentShaderStageInfo.module = fragmentShaderModule;
    FragmentShaderStageInfo.pName = "PSMain";

    VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
    Utils::ZeroVulkanStruct(VertexInputInfo, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);

    VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
    Utils::ZeroVulkanStruct(InputAssembly, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
    InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo ViewportState{};
    Utils::ZeroVulkanStruct(ViewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
    ViewportState.viewportCount = 1;
    ViewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo Rasterizer{};
    Utils::ZeroVulkanStruct(Rasterizer, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
    Rasterizer.depthClampEnable = VK_FALSE;
    Rasterizer.rasterizerDiscardEnable = VK_FALSE;
    Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    Rasterizer.lineWidth = 1.0f;
    Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    Rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    Rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo Multisampling{};
    Utils::ZeroVulkanStruct(Multisampling, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
    Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    Multisampling.sampleShadingEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo DepthStencil{};
    Utils::ZeroVulkanStruct(DepthStencil, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
    DepthStencil.depthTestEnable = VK_TRUE;
    DepthStencil.depthWriteEnable = VK_TRUE;
    DepthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    DepthStencil.depthBoundsTestEnable = VK_FALSE;
    DepthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo ColorBlending{};
    Utils::ZeroVulkanStruct(ColorBlending, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
    ColorBlending.logicOpEnable = VK_FALSE;
    ColorBlending.attachmentCount = 1;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    ColorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineDynamicStateCreateInfo DynamicState{};
    Utils::ZeroVulkanStruct(DynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    DynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    DynamicState.pDynamicStates = dynamicStates.data();
    VkPipelineShaderStageCreateInfo shaderStages[] = { VertexShaderStageInfo, FragmentShaderStageInfo };

    VkFormat colorAttachmentFormat = Swapchain->GetImageFormat();
    VkFormat depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
    Utils::ZeroVulkanStruct(pipelineRenderingInfo, VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO);
    pipelineRenderingInfo.colorAttachmentCount = 1;
    pipelineRenderingInfo.pColorAttachmentFormats = &colorAttachmentFormat;
    pipelineRenderingInfo.depthAttachmentFormat = depthAttachmentFormat;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    Utils::ZeroVulkanStruct(pipelineInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &VertexInputInfo;
    pipelineInfo.pInputAssemblyState = &InputAssembly;
    pipelineInfo.pViewportState = &ViewportState;
    pipelineInfo.pRasterizationState = &Rasterizer;
    pipelineInfo.pMultisampleState = &Multisampling;
    pipelineInfo.pDepthStencilState = &DepthStencil;
    pipelineInfo.pColorBlendState = &ColorBlending;
    pipelineInfo.pDynamicState = &DynamicState;
    pipelineInfo.layout = PipelineLayout;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.pNext = &pipelineRenderingInfo;
    
    if (vkCreateGraphicsPipelines(LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &GraphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(LogicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(LogicalDevice, fragmentShaderModule, nullptr);
}

void FDeviceContext::CreateCommandPool()
{
    VkCommandPoolCreateInfo PoolInfo{};
    Utils::ZeroVulkanStruct(PoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
    PoolInfo.queueFamilyIndex = QueueIndices.GraphicsFamily.value();
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(LogicalDevice, &PoolInfo, nullptr, &CommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void FDeviceContext::CreateCommandBuffers()
{
    CommandBuffers.resize(Swapchain->GetImages().size());
    VkCommandBufferAllocateInfo AllocInfo{};
    Utils::ZeroVulkanStruct(AllocInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    AllocInfo.commandPool = CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = static_cast<uint32_t>(CommandBuffers.size());
    if (vkAllocateCommandBuffers(LogicalDevice, &AllocInfo, CommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void FDeviceContext::RecordCommandBuffers(VkCommandBuffer InCommandBuffer, uint32_t InImageIndex)
{
    VkCommandBufferBeginInfo BeginInfo{};
    Utils::ZeroVulkanStruct(BeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(InCommandBuffer, &BeginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkImageMemoryBarrier2 Barrier{};
    Utils::ZeroVulkanStruct(Barrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2);
    Barrier.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    Barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    Barrier.srcAccessMask = 0;
    Barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    Barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    Barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.image = Swapchain->GetImages()[InImageIndex];
    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.levelCount = 1;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;

    VkDependencyInfo DependencyInfo{};
    Utils::ZeroVulkanStruct(DependencyInfo, VK_STRUCTURE_TYPE_DEPENDENCY_INFO);
    DependencyInfo.imageMemoryBarrierCount = 1;
    DependencyInfo.pImageMemoryBarriers = &Barrier;

    vkCmdPipelineBarrier2(InCommandBuffer, &DependencyInfo);

    VkRenderingAttachmentInfo ColorAttachment{};
    Utils::ZeroVulkanStruct(ColorAttachment, VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO);
    ColorAttachment.imageView = Swapchain->GetImageViews()[InImageIndex];
    ColorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue ClearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    ColorAttachment.clearValue = ClearColor;
    VkRenderingInfo RenderingInfo{};
    Utils::ZeroVulkanStruct(RenderingInfo, VK_STRUCTURE_TYPE_RENDERING_INFO);
    RenderingInfo.renderArea.offset = { 0, 0 };
    RenderingInfo.renderArea.extent = Swapchain->GetExtent();
    RenderingInfo.layerCount = 1;
    RenderingInfo.colorAttachmentCount = 1;
    RenderingInfo.pColorAttachments = &ColorAttachment;
    vkCmdBeginRendering(InCommandBuffer, &RenderingInfo);

    vkCmdBindPipeline(InCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline);

    VkViewport Viewport{};
    Viewport.x = 0.0f;
    Viewport.y = 0.0f;
    Viewport.width = static_cast<float>(Swapchain->GetExtent().width);
    Viewport.height = static_cast<float>(Swapchain->GetExtent().height);
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;
    vkCmdSetViewport(InCommandBuffer, 0, 1, &Viewport);

    VkRect2D Scissor{};
    Scissor.offset = { 0, 0 };
    Scissor.extent = Swapchain->GetExtent();
    vkCmdSetScissor(InCommandBuffer, 0, 1, &Scissor);
    
    vkCmdDraw(InCommandBuffer, 3, 1, 0, 0);

    vkCmdEndRendering(InCommandBuffer);

    VkImageMemoryBarrier2 PresentBarrier = Barrier;
    PresentBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    PresentBarrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

    PresentBarrier.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT; // 或者不用设
    PresentBarrier.dstAccessMask = 0;

    PresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    PresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    DependencyInfo.pImageMemoryBarriers = &PresentBarrier;
    vkCmdPipelineBarrier2(InCommandBuffer, &DependencyInfo);

    if (vkEndCommandBuffer(InCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void FDeviceContext::CreateSyncObjects()
{
    ImageAvailableSemaphores.clear();
    ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkSemaphoreCreateInfo SemaphoreInfo{};
        Utils::ZeroVulkanStruct(SemaphoreInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
        if (vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    size_t ImageCount = Swapchain->GetImages().size();
    PresentSemaphores.resize(ImageCount);

    VkSemaphoreCreateInfo PresentSemaphoreInfo{};
    Utils::ZeroVulkanStruct(PresentSemaphoreInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);

    for (size_t i = 0; i < ImageCount; i++) {
        if (vkCreateSemaphore(LogicalDevice, &PresentSemaphoreInfo, nullptr, &PresentSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create present semaphore!");
        }
    }

    VkSemaphoreTypeCreateInfo TimelineCreateInfo{};
    Utils::ZeroVulkanStruct(TimelineCreateInfo, VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO);
    TimelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    TimelineCreateInfo.initialValue = 0;

    VkSemaphoreCreateInfo SemaphoreInfo{};
    Utils::ZeroVulkanStruct(SemaphoreInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
    SemaphoreInfo.pNext = &TimelineCreateInfo;
    if (vkCreateSemaphore(LogicalDevice, &SemaphoreInfo, nullptr, &GraphicsTimelineSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create timeline semaphore!");
    }
}

bool FDeviceContext::RenderFrame()
{
    uint64_t WaitValue = 0;
    if (CurrentCpuFrame >= MAX_FRAMES_IN_FLIGHT) {
        WaitValue = CurrentCpuFrame + 1 - MAX_FRAMES_IN_FLIGHT; // 当前帧数减去最大帧数，得到需要等待的值

        VkSemaphoreWaitInfo WaitInfo{};
        Utils::ZeroVulkanStruct(WaitInfo, VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO);
        WaitInfo.semaphoreCount = 1;
        WaitInfo.pSemaphores = &GraphicsTimelineSemaphore;
        WaitInfo.pValues = &WaitValue;

        vkWaitSemaphores(LogicalDevice, &WaitInfo, UINT64_MAX);
    }

    uint32_t FrameIndex = CurrentCpuFrame % MAX_FRAMES_IN_FLIGHT;
    uint32_t ImageIndex;
    
    VkResult result = vkAcquireNextImageKHR(LogicalDevice, Swapchain->GetHandle(), UINT64_MAX,
        ImageAvailableSemaphores[FrameIndex], VK_NULL_HANDLE, &ImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
        return false;
    }

    vkResetCommandBuffer(CommandBuffers[ImageIndex], 0);
    RecordCommandBuffers(CommandBuffers[ImageIndex], ImageIndex);

    if (CommandBuffers[ImageIndex] == VK_NULL_HANDLE) {
        throw std::runtime_error("Command Buffer is NULL! Check CreateCommandBuffers.");
    }

    CurrentCpuFrame++;

    VkSemaphoreSubmitInfo WaitBinary{};
    Utils::ZeroVulkanStruct(WaitBinary, VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO);
    WaitBinary.semaphore = ImageAvailableSemaphores[FrameIndex];
    WaitBinary.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSemaphoreSubmitInfo SignalInfos[2];
    Utils::ZeroVulkanStruct(SignalInfos[0], VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO);
    SignalInfos[0].semaphore = GraphicsTimelineSemaphore;
    SignalInfos[0].value = CurrentCpuFrame;
    SignalInfos[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    Utils::ZeroVulkanStruct(SignalInfos[1], VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO);
    SignalInfos[1].semaphore = PresentSemaphores[ImageIndex]; // 当前图片的信号量
    SignalInfos[1].value = 0;
    SignalInfos[1].stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkCommandBufferSubmitInfo CommandBufferInfo{};
    Utils::ZeroVulkanStruct(CommandBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO);
    CommandBufferInfo.commandBuffer = CommandBuffers[ImageIndex];
    
    VkSubmitInfo2 SubmitInfo{};
    Utils::ZeroVulkanStruct(SubmitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO_2);
    SubmitInfo.waitSemaphoreInfoCount = 1;
    SubmitInfo.pWaitSemaphoreInfos = &WaitBinary;
    SubmitInfo.commandBufferInfoCount = 1;
    SubmitInfo.pCommandBufferInfos = &CommandBufferInfo;
    SubmitInfo.signalSemaphoreInfoCount = 2;
    SubmitInfo.pSignalSemaphoreInfos = SignalInfos;

    if (vkQueueSubmit2(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    
    VkPresentInfoKHR PresentInfo{};
    Utils::ZeroVulkanStruct(PresentInfo, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = &PresentSemaphores[ImageIndex];
    
    VkSwapchainKHR Swapchains[] = { Swapchain->GetHandle() };
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = Swapchains;
    PresentInfo.pImageIndices = &ImageIndex;

    result = vkQueuePresentKHR(PresentQueue, &PresentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return false;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
        return false;
    }

    return true;
}
