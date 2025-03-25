#pragma once

#include "vk_types.h"
#include "Buffer.h"
#include <vector>

template<typename T>
struct Cube;

template<typename T>
struct Quad;

class VulkanTutorial;
class VulkanTutorialExtension;
class SimplePipelinePosOnly;
class SimplePipelineTexOnly;

using CubeFrameBuffer = std::vector<std::vector<VkFramebuffer>>;

class IrradianceCubeMap
{
public:
	IrradianceCubeMap(VkDevice inDevice, VkDescriptorPool inDescriptorPool);
	~IrradianceCubeMap() 
	{
		clear();
	}

	void initialize(VulkanTutorialExtension* engine);

	VkImageView getCubeImageView() { return envCubeMap.cubeImageView; }
	VkImageView getDiffuseMapImageView() { return diffuseMap.cubeImageView; }
	VkImageView getSpecularMapImageView() { return specularPrefilteredMap.cubeImageView; }
	VkImageView getSpecularBRDFLUTImageView() { return specularBRDFLUT.imageView; }

	void draw(VkCommandBuffer commandBuffer, VulkanTutorialExtension* engine);
private:
	void createSampler(VkDevice device);
	void loadEquirectangular(VulkanTutorial* engine, const std::string& path);
	void createRenderPass();
	void buildPipeline(VulkanTutorialExtension* engine);
	void clear();

	CubeMap createCubeImage(VulkanTutorial* engine, uint32_t mipLevels, VkExtent2D extent, VkFormat format, const std::string& debugName);
	AllocatedImage create2DImage(VulkanTutorial* engine, uint32_t mipLevels, VkExtent2D extent, VkFormat format, const std::string& debugName);
	void createFrameBuffer(CubeFrameBuffer& frameBuffers, uint32_t mipLevels, const CubeMap& cubeMap, VkExtent2D extent, VkRenderPass renderPass = VK_NULL_HANDLE);
	VkFramebuffer createFrameBuffer2D(const AllocatedImage& image, VkExtent2D extent, VkRenderPass renderPass = VK_NULL_HANDLE);

	std::string equirectangularPath = "textures/brown_photostudio_02_4k.hdr";

	VkFormat HDRFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	VkDevice device;
	VkDescriptorPool descriptorPool;

	VkDescriptorSetLayout layout;
	VkRenderPass renderPass;
	CubeFrameBuffer frameBuffers;

	VkSampler defaultSampler;
	AllocatedImage equirectangularTexture;
	CubeMap envCubeMap;
	std::shared_ptr<SimplePipelinePosOnly> envMapPipeline;
	VkExtent2D Res = { 512, 512 };

	// indirect diffuse map
	CubeMap diffuseMap;
	CubeFrameBuffer diffuseFrameBuffers;
	std::shared_ptr<SimplePipelinePosOnly> diffuseMapPipeline;
	VkExtent2D DiffuseMapRes = { 32, 32 };

	// indirect specular map
	CubeMap specularPrefilteredMap;
	CubeFrameBuffer specularFrameBuffers;
	std::shared_ptr<SimplePipelinePosOnly> specularMapPipeline;
	VkExtent2D SpecularMapRes = { 128, 128 };

	AllocatedImage specularBRDFLUT;
	VkFramebuffer integrationFrameBuffers;
	std::shared_ptr<SimplePipelineTexOnly> integrationMapPipeline;
	VkExtent2D IntegraionMapRes = { 512, 512 };
	VkRenderPass integrationRenderPass;

	const int maxMipLevels = 5;
	const int Faces = 6;

	std::shared_ptr<Cube<VertexOnlyPos>> cube;
	std::shared_ptr<Quad<VertexOnlyTex>> quad;
};