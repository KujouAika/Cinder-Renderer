#pragma once

// 1. 标准库 (这些东西几乎每个文件都要用，且编译极慢，放这里加速)
#include <vector>
#include <string>
#include <memory>
#include <set>
#include <map>
#include <iostream>
#include <algorithm>
#include <functional>
#include <span>
#include <optional>
#include <stdexcept>

// 2. 第三方库基础
#include <vulkan/vulkan.h>

// 3. 基础设施
#include "Config.h"
#include "Macro.h" 
#include "VulkanHandle.h"