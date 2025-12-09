#pragma once
#include <fstream>

namespace Utils
{
    // 读取二进制文件工具函数
    inline std::vector<char> ReadFile(const std::string& filename)
    {
        // ate: 从末尾开始读(为了获取大小) | binary: 二进制模式
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    template< typename T >
    static inline void ZeroVulkanStruct(T& structObj, VkStructureType type)
    {
        structObj = {};
        structObj.sType = type;
        structObj.pNext = nullptr;
    }
}