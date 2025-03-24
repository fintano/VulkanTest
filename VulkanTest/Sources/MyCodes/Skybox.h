#pragma once

#include "vk_types.h"
#include "vk_engine.h"
#include <memory>

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

	RenderObject& getRenderObject() { return renderObject; }

private:
	RenderObject renderObject;
	std::shared_ptr<SimplePipelinePosOnly> pipeline;
	std::shared_ptr<MeshAsset<VertexOnlyPos>> mesh;
};
