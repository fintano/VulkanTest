#pragma once

#include "vk_engine.h"
#include "DeferredDeletionQueue.h"

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
	enum class Model
	{
		Cube,
		Sphere
	};

	void init(VulkanTutorialExtension* engine);
	void createMaterial(VulkanTutorialExtension* engine,
		const std::string& name,
		const std::string& albedoPath,
		const std::string& normalPath,
		const std::string& metallicPath,
		const std::string& roughnessPath,
		const std::string& aoPath
	);
	void selectModel(MaterialTester::Model inModel);
	void draw(DrawContext& mainDrawContext, const std::string& name);
	void cleanUp(VkDevice device);

private:
	Model model = Model::Sphere;
	std::unordered_map<std::string, std::shared_ptr<AllocatedImage>> images;
	std::shared_ptr<Sphere<Vertex>> sphere {};
	std::shared_ptr<Cube<Vertex>> cube {};
	std::unordered_map<std::string /* name */, std::shared_ptr<GLTFMetallic_Roughness::Material>> materialMap;
};