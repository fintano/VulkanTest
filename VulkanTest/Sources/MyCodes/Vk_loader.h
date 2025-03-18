#pragma once

#include <unordered_map>
#include <filesystem>
#include <optional>

#include "vk_types.h"
#include "Buffer.h"

class VulkanTutorial;

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

std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(VulkanTutorial* engine, std::filesystem::path filePath);