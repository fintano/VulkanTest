#pragma once

#include "vk_types.h"

struct MeshAsset;

struct GLTFMetallic_Roughness {
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;

	VkDescriptorSetLayout materialLayout;

	struct MaterialConstants {
		glm::vec4 colorFactors;
		glm::vec4 metal_rough_factors;
		//padding, we need it anyway for uniform buffers
		glm::vec4 extra[14];
	};

	struct MaterialResources {
		//AllocatedImage colorImage;
		VkImageView colorImage;
		VkSampler colorSampler;
		//AllocatedImage metalRoughImage;
		VkImageView metalRoughImage;
		VkSampler metalRoughSampler;
		VkBuffer dataBuffer;
		uint32_t dataBufferOffset;
	};

	//DescriptorWriter writer;

	void build_pipelines(class VulkanTutorialExtension* engine);
	void clear_resources(VkDevice device);

	//MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator);
	MaterialInstance write_material(VulkanTutorialExtension* engine, MaterialPass pass, const MaterialResources& resources, VkDescriptorPool descriptorPool);
};

struct RenderObject {
	uint32_t indexCount;
	uint32_t firstIndex;
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;

	MaterialInstance* material;

	glm::mat4 transform;
};

struct DrawContext {
	std::vector<RenderObject> OpaqueSurfaces;
	std::vector<RenderObject> TranslucentSurfaces;
};

struct MeshNode : public Node {

	std::shared_ptr<MeshAsset> mesh;

	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;
};