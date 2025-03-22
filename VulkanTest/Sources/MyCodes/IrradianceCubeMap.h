#pragma once

#include "vk_types.h"

class IrradianceCubeMap
{
	IrradianceCubeMap(const AllocatedImage& inEquirectangular);
	~IrradianceCubeMap();

private:
	void createRenderPass(VkDevice device);
	void buildPipeline(VkDevice device, VkDescriptorSetLayout globalLayout);
	void clear();

private:
	const AllocatedImage& equirectangular;
	MaterialPipeline pipeline;
	VkDescriptorSetLayout layout;
	VkRenderPass renderPass;

	VkExtent2D faceRes = { 512, 512 };
};