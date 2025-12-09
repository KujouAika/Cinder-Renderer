#pragma once
#include <fstream>
#include <filesystem>

namespace Utils
{
    // 读取二进制文件工具函数
    inline std::vector<char> ReadSPV(const std::string& filename)
    {
        std::filesystem::path rootPath = SHADER_ROOT;
        std::filesystem::path fullPath = rootPath / filename;
        std::ifstream file(fullPath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + fullPath.string());
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