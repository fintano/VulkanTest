#pragma once
#include <string>
#include <filesystem>
#include <vulkan/vulkan.h>

typedef unsigned char stbi_uc;

namespace fastgltf {
    template<typename T>
    class Expected;

    class GltfDataBuffer;
};

namespace Utils {
    enum
    {
        STBI_default = 0, // only used for desired_channels

        STBI_grey = 1,
        STBI_grey_alpha = 2,
        STBI_rgb = 3,
        STBI_rgb_alpha = 4
    };

    VkShaderModule loadShader(const std::string& shaderName, VkDevice device);
    float* loadImagef(char const* filename, int* x, int* y, int* comp, int req_comp);
    stbi_uc* loadImage(char const* filename, int* x, int* y, int* comp, int req_comp);
    void freeImage(stbi_uc* data);
    fastgltf::Expected<fastgltf::GltfDataBuffer> loadModel(const std::string_view& filePath);
};