#pragma once

#include "vk_types.h"
#include "Vertex.h"
#include "UniformBuffer.h"

#include <unordered_map>

template<typename T>
struct MeshAsset;

struct GLTFMetallic_Roughness {
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;

	VkDescriptorSetLayout materialLayout;

	struct MaterialConstants {
		glm::vec4 colorFactors = glm::vec4(1.f);
		glm::vec4 metal_rough_factors = glm::vec4(1.f); // x=metallicFactor, y=roughnessFactor, z= normalScale, a=occulusionStrength
		glm::vec4 textureFlags; // x=useNormalMap, y=useMetallicMap, z=useRoughnessMap, w=useAOMap
		glm::vec4 emissiveFactors = glm::vec4(0.f);
		//padding, we need it anyway for uniform buffers
		glm::vec4 extra[12];
	};


	// 소유권 없음
	struct MaterialResources {
		std::shared_ptr<AllocatedImage> colorImage;
		std::shared_ptr<AllocatedImage> metalRoughImage;
		std::shared_ptr<AllocatedImage> normalImage;
		std::shared_ptr<AllocatedImage> metallicImage;
		std::shared_ptr<AllocatedImage> roughnessImage;
		std::shared_ptr<AllocatedImage> AOImage;
		std::shared_ptr<AllocatedImage> emissiveImage;

		VkSampler colorSampler;
		VkSampler metalRoughSampler;
		VkBuffer dataBuffer;
		uint32_t dataBufferOffset;
	};

	struct Material
	{
		Material()
		{
			constants = UniformBuffer<GLTFMetallic_Roughness::MaterialConstants>::create();
			resources = {};
		}
		std::shared_ptr<UniformBuffer<GLTFMetallic_Roughness::MaterialConstants>> constants;
		GLTFMetallic_Roughness::MaterialResources resources;
		std::shared_ptr<MaterialInstance> materialInstances;
	};

	void build_pipelines(class VulkanTutorialExtension* engine);
	void clear_resources(VkDevice device);

	std::shared_ptr<MaterialInstance> write_material(VulkanTutorialExtension* engine, MaterialPass pass, const MaterialResources& resources, VkDescriptorPool descriptorPool);
	std::shared_ptr<GLTFMetallic_Roughness::Material> create_material_resources(VulkanTutorialExtension* engine, std::shared_ptr<AllocatedImage>& color, std::shared_ptr<AllocatedImage>& normal, std::shared_ptr<AllocatedImage>& metallic, std::shared_ptr<AllocatedImage>& roughness, std::shared_ptr<AllocatedImage>& AO, glm::vec4 textureFlags);
};

struct RenderObject {
	uint32_t indexCount;
	uint32_t firstIndex;
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;

	std::shared_ptr<MaterialInstance> material;

	glm::mat4 transform;
};

struct DrawContext {
	std::vector<RenderObject> OpaqueSurfaces;
	std::vector<RenderObject> TranslucentSurfaces;
};

struct MeshNode : public Node {

	std::shared_ptr<MeshAsset<Vertex>> mesh;

	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override;
};