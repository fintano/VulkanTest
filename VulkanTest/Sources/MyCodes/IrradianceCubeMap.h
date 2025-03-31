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
class SimplePipelinePosTex;

using CubeFrameBuffer = std::vector<std::vector<VkFramebuffer>>;

class IrradianceCubeMap
{
public:
	IrradianceCubeMap(DevicePtr inDevice, VkDescriptorPool inDescriptorPool);
	~IrradianceCubeMap() 
	{
		clear();
	}

	void initialize(VulkanTutorialExtension* engine);

	std::shared_ptr<AllocatedImage> getCubeImageView() { return envCubeMap->image; }
	std::shared_ptr<AllocatedImage> getDiffuseMapImageView() { return diffuseMap->image; }
	std::shared_ptr<AllocatedImage> getSpecularMapImageView() { return specularPrefilteredMap->image; }
	std::shared_ptr<AllocatedImage> getSpecularBRDFLUTImageView() { return specularBRDFLUT; }

	void draw(VkCommandBuffer commandBuffer, VulkanTutorialExtension* engine);
private:
	void createSampler(VkDevice device);
	void loadEquirectangular(VulkanTutorial* engine, const std::string& path);
	void createRenderPass();
	void buildPipeline(VulkanTutorialExtension* engine);
	void clear();

	std::shared_ptr<CubeMap> createCubeImage(VulkanTutorial* engine, uint32_t mipLevels, VkExtent2D extent, VkFormat format, const std::string& debugName);
	std::shared_ptr<AllocatedImage> create2DImage(VulkanTutorial* engine, uint32_t mipLevels, VkExtent2D extent, VkFormat format, const std::string& debugName);
	void createFrameBuffer(CubeFrameBuffer& frameBuffers, uint32_t mipLevels, const CubeMap& cubeMap, VkExtent2D extent, VkRenderPass renderPass = VK_NULL_HANDLE);
	VkFramebuffer createFrameBuffer2D(const std::shared_ptr<AllocatedImage>& image, VkExtent2D extent, VkRenderPass renderPass = VK_NULL_HANDLE);

	std::string equirectangularPath = "textures/newport_loft.hdr";

	VkFormat HDRFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

	DevicePtr device;
	VkDescriptorPool descriptorPool;

	VkDescriptorSetLayout layout;
	VkRenderPass renderPass;
	CubeFrameBuffer frameBuffers;

	VkSampler defaultSampler;
	std::shared_ptr<AllocatedImage> equirectangularTexture;
	std::shared_ptr<CubeMap> envCubeMap;
	std::shared_ptr<SimplePipelinePosOnly> envMapPipeline;
	VkExtent2D Res = { 512, 512 };
	VkRenderPass envCubeRenderPass;

	// indirect diffuse map
	std::shared_ptr<CubeMap> diffuseMap;
	CubeFrameBuffer diffuseFrameBuffers;
	std::shared_ptr<SimplePipelinePosOnly> diffuseMapPipeline;
	VkExtent2D DiffuseMapRes = { 32, 32 };

	// indirect specular map
	std::shared_ptr<CubeMap> specularPrefilteredMap;
	CubeFrameBuffer specularFrameBuffers;
	std::shared_ptr<SimplePipelinePosOnly> specularMapPipeline;
	VkExtent2D SpecularMapRes = { 128, 128 };

	std::shared_ptr<AllocatedImage> specularBRDFLUT;
	VkFramebuffer integrationFrameBuffers;
	std::shared_ptr<SimplePipelinePosTex> integrationMapPipeline;
	VkExtent2D IntegraionMapRes = { 512, 512 };
	VkRenderPass integrationRenderPass;

	const int maxMipLevels = 5;
	const int Faces = 6;
	const int envMipLevels;

	std::shared_ptr<Cube<VertexOnlyPos>> cube;
	std::shared_ptr<Quad<VertexOnlyTex>> quad;
};