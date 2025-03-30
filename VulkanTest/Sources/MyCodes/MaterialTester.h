#pragma once

#include "vk_engine.h"

#include <memory>

template<typename T>
struct Sphere;

template<typename T>
struct Cube;

class VulkanTutorialExtension;
class DrawContext;
struct Vertex;

class MaterialTester
{
public:
	void init(VulkanTutorialExtension* engine);
	void createMaterial(VulkanTutorialExtension* engine,
		const std::string& name,
		const char* albedoPath,
		const char* normalPath,
		const char* metallicPath,
		const char* roughnessPath,
		const char* aoPath
	);
	void draw(DrawContext& mainDrawContext, const std::string& name);
	void cleanUp(VkDevice device);

private:
	std::vector<AllocatedImage> images;
	std::shared_ptr<Sphere<Vertex>> sphere {};
	std::shared_ptr<Cube<Vertex>> cube {};
	std::unordered_map<std::string /* name */, GLTFMetallic_Roughness::Material> materialMap;
};