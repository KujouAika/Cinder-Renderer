cmake_minimum_required(VERSION 3.25)

# ==============================================================================
# 1. 项目基础配置
# ==============================================================================
project(VulkanRenderer CXX)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ==============================================================================
# 2. 变量定义 (统一管理路径)
# ==============================================================================
set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/Shaders")

# ==============================================================================
# 3. 寻找依赖环境
# ==============================================================================
find_package(Vulkan REQUIRED)

if (NOT DEFINED ENV{VULKAN_SDK})
    message(FATAL_ERROR "VULKAN_SDK environment variable is not set.")
endif()

file(TO_CMAKE_PATH "$ENV{VULKAN_SDK}" VULKAN_SDK_PATH)

# DXC 编译器
find_program(DXC_EXECUTABLE dxc
    HINTS "${VULKAN_SDK_PATH}/Bin"
    DOC "Path to DirectX Shader Compiler"
)
if(NOT DXC_EXECUTABLE)
    message(FATAL_ERROR "Could not find dxc.exe!")
endif()

# ==============================================================================
# 4. 预处理 (创建文件夹)
# ==============================================================================
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

# ==============================================================================
# 5. 函数定义
# ==============================================================================
function(compile_shader SHADER_SOURCE PROFILE ENTRY_POINT OUTPUT_FILENAME)
    # 拼接完整输出路径
    set(FULL_OUTPUT_PATH "${SHADER_OUTPUT_DIR}/${OUTPUT_FILENAME}")

    add_custom_command(
        OUTPUT ${FULL_OUTPUT_PATH}
        # 【关键】使用 -Fo 指定完整路径
        COMMAND ${DXC_EXECUTABLE} -T ${PROFILE} -E ${ENTRY_POINT} -Fo ${FULL_OUTPUT_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE} -spirv
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SOURCE}
        COMMENT "Compiling HLSL: ${SHADER_SOURCE} -> ${OUTPUT_FILENAME}"
    )

    # 将生成的完整路径添加到列表
    list(APPEND ALL_GENERATED_SPV_FILES ${FULL_OUTPUT_PATH})
    set(ALL_GENERATED_SPV_FILES ${ALL_GENERATED_SPV_FILES} PARENT_SCOPE)
endfunction()

# ==============================================================================
# 6. Shader 编译任务定义
# ==============================================================================
compile_shader("Shaders/Triangle.hlsl" "vs_6_0" "VSMain" "Triangle.vert.spv")
compile_shader("Shaders/Triangle.hlsl" "ps_6_0" "PSMain" "Triangle.frag.spv")

# Target 管理 Shader 任务
add_custom_target(CompileShaders ALL DEPENDS ${ALL_GENERATED_SPV_FILES})

# ==============================================================================
# 7. 定义 C++ 源码
# ==============================================================================
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

# 使用 source_group 整理 VS 中的目录结构
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES 
    ${SRC_APP} ${SRC_RHI} ${SRC_CORE} src/main.cpp)

# ==============================================================================
# 8. 构建主程序 (Executable)
# ==============================================================================
add_executable(${PROJECT_NAME} 
    src/main.cpp
    ${SRC_APP}
    ${SRC_RHI}
    ${SRC_CORE}
)

# ==============================================================================
# 9. 配置目标属性 (Include, Link, Defines, Dependencies)
# ==============================================================================
# 确保先编译 Shader 再链接程序
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
    "${VULKAN_SDK_PATH}/Lib/SDL2.lib"
    "${VULKAN_SDK_PATH}/Lib/SDL2main.lib"
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    src/Core/Common.h
)

target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    "SHADER_ROOT=\"${SHADER_OUTPUT_DIR}\""
)