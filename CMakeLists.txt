cmake_minimum_required(VERSION 3.25)


project(VulkanRenderer CXX)


if(MSVC)
    # 强制 MSVC 编译器使用 UTF-8 编码读取源码，并输出 UTF-8 格式的日志
    add_compile_options(/utf-8)
endif()


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


# 尝试寻找 Vulkan SDK 里的 DXC
find_program(DXC_EXECUTABLE dxc
    HINTS "${VULKAN_SDK_PATH}/Bin"
    DOC "Path to DirectX Shader Compiler"
)


if(NOT DXC_EXECUTABLE)
    message(FATAL_ERROR "Could not find dxc.exe! Please install Vulkan SDK.")
endif()


# 定义一个宏，用于编译 HLSL 到 SPIR-V
# 参数:
#   SHADER_SOURCE: 源代码路径 (如 Shaders/Triangle.hlsl)
#   PROFILE:       编译目标 (如 vs_6_0, ps_6_0)
#   ENTRY_POINT:   入口函数 (如 VSMain)
#   OUTPUT_SPV:    输出文件名 (如 Triangle.vert.spv)
function(compile_shader SHADER_SOURCE PROFILE ENTRY_POINT OUTPUT_SPV)
    # 定义输出文件的完整路径 (放到构建目录的 bin 下，或者和 exe 同级)
    set(SPV_FILE "${CMAKE_BINARY_DIR}/${OUTPUT_SPV}")

    # 添加自定义命令
    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${DXC_EXECUTABLE} -T ${PROFILE} -E ${ENTRY_POINT} -Fo ${SPV_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE} -spirv
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE}
        COMMENT "Compiling HLSL shader: ${SHADER_SOURCE} -> ${OUTPUT_SPV}"
    )

    # 将生成的 .spv 文件添加到一个列表中，稍后用来创建 Target
    list(APPEND ALL_GENERATED_SPV_FILES ${SPV_FILE})
    # 因为是在函数里，需要将列表变量向上传递到父作用域
    set(ALL_GENERATED_SPV_FILES ${ALL_GENERATED_SPV_FILES} PARENT_SCOPE)
endfunction()

compile_shader("Shaders/Triangle.hlsl" "vs_6_0" "VSMain" "Triangle.vert.spv")
compile_shader("Shaders/Triangle.hlsl" "ps_6_0" "PSMain" "Triangle.frag.spv")

# 创建一个自定义 Target 叫 "CompileShaders"
# 这样 VS 解决方案里会多出一个项目，专门管 Shader 编译
add_custom_target(CompileShaders ALL DEPENDS ${ALL_GENERATED_SPV_FILES})


set(SRC_APP
    src/Application/Application.h
    src/Application/Application.cpp
    src/Application/Window.h
    src/Application/Window.cpp
)

set(SRC_RHI
    src/RHI/DeviceContext.h
    src/RHI/DeviceContext.cpp
    src/RHI/Swapchain.h
    src/RHI/Swapchain.cpp
    src/RHI/DeviceSelector.h
    src/RHI/DeviceSelector.cpp
)

set(SRC_CORE
    src/Core/VulkanHandle.h
    src/Core/Macro.h
    src/Core/Common.h
    src/Core/Config.h
    src/Core/Utils.h
)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    ${SRC_APP}
    ${SRC_RHI}
    ${SRC_CORE}
)

add_dependencies(${PROJECT_NAME} CompileShaders)

target_include_directories(${PROJECT_NAME} PRIVATE 
    src
    src/Core
    src/RHI
    src/Application
    src/ThirdParty/VMA
    "${VULKAN_SDK_PATH}/Include"
)


target_link_libraries(${PROJECT_NAME} PRIVATE 
    Vulkan::Vulkan 
    "${VULKAN_SDK_PATH}/Lib/SDL2.lib"      # 链接 SDL2.lib
    "${VULKAN_SDK_PATH}/Lib/SDL2main.lib" # 链接 SDL2main.lib
)

# 预编译头 (PCH)
# REUSE_FROM: 如果有多个目标，可以复用 PCH
target_precompile_headers(${PROJECT_NAME} PRIVATE
    src/Core/Common.h
)

# 7. 预处理器定义
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)

# -----------------------------------------------------------------

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${SRC_APP} ${SRC_RHI} ${SRC_CORE})