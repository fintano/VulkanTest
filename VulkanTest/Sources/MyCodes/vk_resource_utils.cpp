#include "vk_resource_utils.h"
#include "vk_pathes.h"
#include "VulkanTools.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/gtx/quaternion.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

namespace Utils {
    VkShaderModule loadShader(const std::string& shaderName, VkDevice device) {
        std::filesystem::path shaderPath = Utils::GetProjectRoot() / shaderName;
        return vks::tools::loadShader(shaderPath.string().c_str(), device);
    }

    float* loadImagef(char const* filename, int* x, int* y, int* comp, int req_comp) {
        std::filesystem::path shaderPath = Utils::GetProjectRoot() / filename;
        return stbi_loadf(shaderPath.string().c_str(), x, y, comp, req_comp);
    }

    stbi_uc* loadImage(char const* filename, int* x, int* y, int* comp, int req_comp) {
        std::filesystem::path shaderPath = Utils::GetProjectRoot() / filename;
        return stbi_load(shaderPath.string().c_str(), x, y, comp, req_comp);
    }

    void freeImage(stbi_uc* data)
    {
        stbi_image_free(data);
    }

    fastgltf::Expected<fastgltf::GltfDataBuffer> loadModel(const std::string_view& filePath)
    {
        std::filesystem::path modelPath = Utils::GetProjectRoot() / std::string(filePath);
        // std::filesystem::path::preferred_separator를 사용하여 시스템 기본 구분자로 변환
        std::string pathStr = modelPath.string();
        for (char& c : pathStr) {
            if (c == '\\') c = '/';
        }

        std::cerr << "loading Model... " << pathStr << std::endl;

        auto result = fastgltf::GltfDataBuffer::FromPath(pathStr);
        if (!result) {
            std::cerr << "Failed to load model: " << pathStr << std::endl;
            std::cerr << "Error: " << (std::uint64_t)(result.error()) << std::endl;
        }

        return result;
    }
}