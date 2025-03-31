#pragma once

#include "vk_types.h"
#include "vk_engine.h"
#include <memory>

template<typename T>
struct Cube;

template<typename T>
struct MeshAsset;
struct DrawContext;
struct VertexOnlyPos;
class VulkanTutorialExtension;
class SimplePipelinePosOnly;

class Skybox
{
public:
	void initialize(VulkanTutorialExtension* engine);
	void update(uint32_t currentImage);
	void cleanup(VkDevice device);
	bool isValid();

	RenderObject& getRenderObject() { return renderObject; }

private:
	RenderObject renderObject;
	std::shared_ptr<SimplePipelinePosOnly> pipeline;
	std::shared_ptr<Cube<VertexOnlyPos>> cube;
	std::shared_ptr<MeshAsset<VertexOnlyPos>> mesh;
	std::shared_ptr<AllocatedImage> irradianceCubeMap;
};
