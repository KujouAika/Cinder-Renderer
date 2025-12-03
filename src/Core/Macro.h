#pragma once
// 1. 平台检测 & 中断指令
// -----------------------------------------------------------------------------
#if defined(_MSC_VER)
    // Windows / Visual Studio
#define CA_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    // Linux / Mac / Clang
#define CA_DEBUG_BREAK() __builtin_trap()
#else
#define CA_DEBUG_BREAK()
#endif

// 2. 确定是否启用断言
// -----------------------------------------------------------------------------
// 如果定义了 _DEBUG，或者没有定义 NDEBUG，我们就开启检查
#if !defined(CA_ENABLE_ASSERTS)
#if defined(_DEBUG) || defined(DEBUG)
#define CA_ENABLE_ASSERTS 1
#else
#define CA_ENABLE_ASSERTS 0
#endif
#endif

// 3. 核心断言宏
// -----------------------------------------------------------------------------
#if CA_ENABLE_ASSERTS

    // 内部函数：打印错误信息
    // 这里的 do-while(0) 是为了让宏在 if-else 语句中安全使用
#define CA_INTERNAL_CHECK_IMPL(expr, file, line) \
        do { \
            if (!(expr)) { \
                std::cerr << "*************************" << std::endl; \
                std::cerr << "!!! ASSERTION FAILED !!!" << std::endl; \
                std::cerr << "File: " << file << std::endl; \
                std::cerr << "Line: " << line << std::endl; \
                std::cerr << "Condition: " << #expr << std::endl; \
                std::cerr << "*************************" << std::endl; \
                CA_DEBUG_BREAK(); \
            } \
        } while(0)

    // 暴露给用户的宏
#define check(expr) CA_INTERNAL_CHECK_IMPL(expr, __FILE__, __LINE__)
#define verify(expr) check(expr)

#else

    // Release 模式：check 直接消失，verify 仅保留执行
#define check(expr) 
#define verify(expr) (expr)

#endif

// 4. Vulkan 专用检查宏
// -----------------------------------------------------------------------------
#if CA_ENABLE_ASSERTS

#define VK_CHECK(expr) \
        do { \
            VkResult result = expr; \
            if (result != VK_SUCCESS) { \
                std::cerr << "*************************" << std::endl; \
                std::cerr << "!!! VULKAN ERROR !!!" << std::endl; \
                std::cerr << "File: " << __FILE__ << std::endl; \
                std::cerr << "Line: " << __LINE__ << std::endl; \
                std::cerr << "Result Code: " << result << std::endl; \
                std::cerr << "Call: " << #expr << std::endl; \
                std::cerr << "*************************" << std::endl; \
                CA_DEBUG_BREAK(); \
            } \
        } while(0)

#else
    // Release 模式下，我们依然要执行 expr（因为它是创建资源的函数），但不检查返回值
    // 或者你可以选择保留检查但只打印日志不中断，这里为了性能选择仅执行
#define VK_CHECK(expr) (expr)
#endif