#pragma once

#include "vk_types.h"

class SimplePipelineEmptyInput;
class VulkanTutorialExtension;

class TextureViewer
{
public:

	~TextureViewer();

	struct TextureInfo {
		std::shared_ptr<CubeMap> cubeMap;
		std::shared_ptr<AllocatedImage> image;
		std::string name;
	};

	void initialize(VulkanTutorialExtension* engine);
	void draw(VkCommandBuffer commandBuffer, VulkanTutorialExtension* engine, size_t index);
	void addTexture(const std::shared_ptr<CubeMap>& cubeMap, const std::string& name);
	void addTexture(const std::shared_ptr<AllocatedImage>& image, const std::string& name);
	void selectTexture(int index);
	void selectTexture(const std::string& name);
	void selectMipLevel(int mipLevel);
	void selectCubeMapFace(int Face);
	const std::vector<TextureInfo>& getTextures() const { return textures; }
	int getSelectedTextureIndex() const { return targetTextureIndex; }
	bool IsChanged();
	void ResetChanged();

private:
	DevicePtr device;

	void createPipeline(VulkanTutorialExtension* engine);
	void updateTextureIfNeeded(VulkanTutorialExtension* engine, size_t index);

	std::vector<TextureInfo> textures;
	std::shared_ptr<SimplePipelineEmptyInput> pipeline;

	int targetTextureIndex = 0; 
	int selectedMipLevel = 0;
	int selectedCubeMapFace = 0;
	bool changed = false;
	std::vector<int> lastUpdatedIndex;
	std::vector<int> lastUpdatedMipLevel;
	std::vector<int> lastUpdatedCubeMapFace;
};