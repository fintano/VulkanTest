#pragma once

#include "vk_types.h"
#include "Buffer.h"
#include <vector>

class VulkanTutorial;
class VulkanTutorialExtension;

class IrradianceCubeMap
{
public:
	IrradianceCubeMap(VkDevice inDevice, VkDescriptorPool inDescriptorPool);
	~IrradianceCubeMap() 
	{
		clear();
	}

	void initialize(VulkanTutorialExtension* engine);
	void createCubeMap(VulkanTutorial* engine);
	void createSampler(VkDevice device);
	void loadEquirectangular(VulkanTutorial* engine, const std::string& path);
	void createRenderPass();
	void createFrameBuffers();
	void buildPipeline();
	void writeDescriptor();
	void draw(VkCommandBuffer commandBuffer, const GPUMeshBuffers<VertexOnlyPos>& mesh);
	void clear();

	VkImageView getCubeImageView() { return envCubeMap.cubeImageView; }
	VkImageView getDiffuseMapImageView() { return diffuseMap.cubeImageView; }

private:
	std::string equirectangularPath = "textures/brown_photostudio_02_4k.hdr";

	VkFormat HDRFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	VkDevice device;
	VkDescriptorPool descriptorPool;

	MaterialPipeline pipeline = {};
	VkDescriptorSetLayout layout = {};
	VkDescriptorSet descriptorSet = {};
	VkRenderPass renderPass = {};
	std::vector<VkFramebuffer> frameBuffers = {};

	VkSampler defaultSampler;
	AllocatedImage equirectangularTexture;
	CubeMap envCubeMap;

	// indirect diffuse map
	MaterialPipeline diffusePipeline = {};
	VkDescriptorSet diffuseDescriptorSet = {};
	CubeMap diffuseMap;
	std::vector<VkFramebuffer> diffuseFrameBuffers = {};

	// indirect specular map
	MaterialPipeline specularPipeline = {};
	VkDescriptorSet specularDescriptorSet = {};
	CubeMap specularMap;
	std::vector<VkFramebuffer> FrameBuffers = {};

	const int Faces = 6;
	VkExtent3D Res = { 512, 512, 6 };
	VkExtent3D DiffuseMapRes = { 32, 32, 6 };
};