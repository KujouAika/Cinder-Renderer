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

# 4. 【关键修正】先创建可执行文件 (Target)
add_executable(${PROJECT_NAME} src/main.cpp)


target_sources(${PROJECT_NAME} PRIVATE
    src/Core/Window.h
    src/Core/Window.cpp
    src/Core/DeviceContext.h
    src/Core/DeviceContext.cpp
)

target_sources(${PROJECT_NAME} PRIVATE
    src/Application.h
    src/Application.cpp
)


target_include_directories(${PROJECT_NAME} PRIVATE 
    src
    "${VULKAN_SDK_PATH}/Include"
)


target_link_libraries(${PROJECT_NAME} PRIVATE 
    Vulkan::Vulkan 
    "${VULKAN_SDK_PATH}/Lib/SDL2.lib"      # 链接 SDL2.lib
    "${VULKAN_SDK_PATH}/Lib/SDL2main.lib" # 链接 SDL2main.lib
)


# 7. 预处理器定义
target_compile_definitions(${PROJECT_NAME} PRIVATE VK_USE_PLATFORM_WIN32_KHR)


# -----------------------------------------------------------------
# 3. (可选但强烈推荐) Visual Studio 过滤器整理
# Senior 思维：你的 Solution Explorer 不能是一团乱麻。
# 手动通过 source_group 让 VS 里的文件结构和硬盘文件夹保持一致。
# -----------------------------------------------------------------
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES
    src/Core/Window.h
    src/Core/Window.cpp
    src/Core/DeviceContext.h
    src/Core/DeviceContext.cpp
    src/Application.h
    src/Application.cpp
)