#pragma once

#include "vk_types.h"
#include "Vk_loader.h"

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
	MaterialInstance write_material(VulkanTutorialExtension* engine, MaterialPass pass, const MaterialResources& resources);
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
};

struct MeshNode : public Node {

	std::shared_ptr<struct MeshAsset> mesh;

	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override
	{
		glm::mat4 nodeMatrix = topMatrix * worldTransform;

		for (auto& s : mesh->surfaces) {
			RenderObject def;
			def.indexCount = s.count;
			def.firstIndex = s.startIndex;
			def.vertexBuffer = mesh->meshBuffers.vertexBuffer.Buffer;
			def.indexBuffer = mesh->meshBuffers.indexBuffer.Buffer;
			def.material = &s.material->data;

			def.transform = nodeMatrix;
			def.vertexBuffer = mesh->meshBuffers.vertexBuffer.Buffer;

			ctx.OpaqueSurfaces.push_back(def);
		}

		// recurse down
		Node::Draw(topMatrix, ctx);
	}
};