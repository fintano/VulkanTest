#pragma once

#include <unordered_map>
#include <filesystem>
#include <optional>

#include "vk_types.h"
#include "vk_engine.h"
#include "UniformBuffer.h"
#include "Buffer.h"

class VulkanTutorialExtension;

struct GLTFMaterial {
	MaterialInstance data;
};

struct GeoSurface 
{
	uint32_t startIndex;
	uint32_t count;
	std::shared_ptr<GLTFMaterial> material;
};

struct MeshAsset
{
	std::string name;

	std::vector<GeoSurface> surfaces;
	GPUMeshBuffers meshBuffers;
};

struct LoadedGLTF : public IRenderable {

    // storage for all the data on a given glTF file
    std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
    std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
    std::unordered_map<std::string, AllocatedImage> images;
    std::unordered_map<std::string, std::shared_ptr<GLTFMaterial>> materials;

    // nodes that dont have a parent, for iterating through the file in tree order
    std::vector<std::shared_ptr<Node>> topNodes;

    std::vector<VkSampler> samplers;

    VkDescriptorPool descriptorPool;

    UniformBuffer<GLTFMetallic_Roughness::MaterialConstants> materialDataBuffer;

    VulkanTutorialExtension* creator;

    ~LoadedGLTF() { clearAll(); };

    virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx);

private:

    void clearAll();
};

std::optional<std::shared_ptr<LoadedGLTF>> loadGltf(VulkanTutorialExtension* engine, std::string_view filePath);