#pragma once
#include <vulkan/vulkan.h>

// 全局统一的 Vulkan API 版本
// 修改此处将影响 Instance 创建、Device 选择以及 VMA 初始化
constexpr uint32_t VK_API_VERSION = VK_API_VERSION_1_3;

// 是否启用验证层 (Debug 模式下)
#ifdef NDEBUG
constexpr bool bEnableValidationLayers = false;
#else
constexpr bool bEnableValidationLayers = true;
#endif

// 应用名称与引擎版本
constexpr const char* APP_NAME = "Cinder Renderer";
constexpr uint32_t APP_VERSION = VK_MAKE_VERSION(2, 5, 0);
constexpr const char* ENGINE_NAME = "Cinder Engine";
constexpr uint32_t ENGINE_VERSION = VK_MAKE_VERSION(16, 0, 0);

// 最大同时帧数 (Frames In Flight)
const int MAX_FRAMES_IN_FLIGHT = 2;