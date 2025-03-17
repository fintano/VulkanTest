#pragma once

#include "vk_types.h"
#include "UniformBuffer.h"

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
		Texture colorImage;
		VkSampler colorSampler;
		Texture metalRoughImage;
		VkSampler metalRoughSampler;
		
		UniformBuffer<MaterialConstants> dataBuffer;
		//VkBuffer dataBuffer;
		//uint32_t dataBufferOffset;
	};

	//DescriptorWriter writer;

	void build_pipelines(class VulkanTutorial* engine);
	void clear_resources(VkDevice device);

	MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator);
};

struct RenderObject {
	uint32_t indexCount;
	uint32_t firstIndex;
	VkBuffer indexBuffer;

	MaterialInstance* material;

	glm::mat4 transform;
	VkDeviceAddress vertexBufferAddress;
};

struct DrawContext {
	std::vector<RenderObject> OpaqueSurfaces;
};