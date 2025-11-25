cmake_minimum_required(VERSION 3.25)

project(VulkanRenderer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 1. 查找 Vulkan (这会定义 Vulkan::Vulkan 和 Vulkan_GLSLC_EXECUTABLE)
find_package(Vulkan REQUIRED)

# 2. 检查环境变量
if (NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is not set. Please install Vulkan SDK.")
endif()

# 3. 路径转换与变量定义
file(TO_CMAKE_PATH "$ENV{VULKAN_SDK}" VULKAN_SDK_PATH)

# shader 编译函数 (保持不变)
function(compile_shader SHADER_SOURCE SHADER_BINARY)
    add_custom_command(
        OUTPUT ${SHADER_BINARY}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER_SOURCE} -o ${SHADER_BINARY}
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling ${SHADER_SOURCE} to SPIR-V"
    )
endfunction()

# 4. 【关键修正】先创建可执行文件 (Target)
add_executable(${PROJECT_NAME} src/main.cpp)

# 5. 配置头文件包含路径 (Include Directories)
# 必须把 SDL2 和 GLM 的头文件路径告诉编译器
target_include_directories(${PROJECT_NAME} PRIVATE 
    src
    "${VULKAN_SDK_PATH}/Include"
)

# 6. 【关键修正】链接库文件 (Link Libraries)
# 必须在 add_executable 之后调用
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Vulkan::Vulkan 
    "${VULKAN_SDK_PATH}/Lib/SDL2.lib"      # 链接 SDL2.lib
    "${VULKAN_SDK_PATH}/Lib/SDL2main.lib" # 链接 SDL2main.lib
)

# 7. 预处理器定义
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)