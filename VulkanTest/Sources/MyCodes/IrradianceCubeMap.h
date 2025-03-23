#pragma once

#include "vk_types.h"
#include "Buffer.h"
#include <vector>

class VulkanTutorial;
class VulkanTutorialExtension;

class IrradianceCubeMap
{
public:
	IrradianceCubeMap(VkDevice inDevice, VkDescriptorPool inDescriptorPool, VkSampler inDefaultSampler);
	~IrradianceCubeMap() 
	{
		clear();
	}

	void initialize(VulkanTutorialExtension* engine);
	void createCubeMap(VulkanTutorial* engine);
	void loadEquirectangular(VulkanTutorial* engine, const std::string& path);
	void createRenderPass();
	void createFrameBuffers();
	void buildPipeline();
	void writeDescriptor();
	void draw(VkCommandBuffer commandBuffer, const GPUMeshBuffers<VertexOnlyPos>& mesh);
	void clear();

	VkImageView getCubeImageView() { return cubeImageView; }

private:
	std::string equirectangularPath = "textures/photo_studio_loft_hall_4k.hdr";

	VkFormat HDRFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	VkDevice device;
	VkDescriptorPool descriptorPool;

	MaterialPipeline pipeline = {};
	VkDescriptorSetLayout layout = {};
	VkDescriptorSet descriptorSet = {};
	VkRenderPass renderPass = {};
	std::vector<VkFramebuffer> frameBuffers = {};

	AllocatedImage equirectangularTexture;

	VkImage image;
	VkDeviceMemory imageMemory;
	std::vector<VkImageView> imageViews;
	VkImageView cubeImageView;
	VkSampler defaultSampler;

	const int Faces = 6;
	VkExtent3D Res = { 512, 512, 6 };
};