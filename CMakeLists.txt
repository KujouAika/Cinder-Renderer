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


# shader 编译函数 (保持不变)
function(compile_shader SHADER_SOURCE SHADER_BINARY)
    add_custom_command(
        OUTPUT ${SHADER_BINARY}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER_SOURCE} -o ${SHADER_BINARY}
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling ${SHADER_SOURCE} to SPIR-V"
    )
endfunction()

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
)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    ${SRC_APP}
    ${SRC_RHI}
    ${SRC_CORE}
)


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
    src/Core/Config.h
)

# 7. 预处理器定义
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)

# -----------------------------------------------------------------

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${SRC_APP} ${SRC_RHI} ${SRC_CORE})