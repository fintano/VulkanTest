#include "TextureViewer.h"
#include "VulkanTutorialExtension.h"
#include "SimplePipeline.h"
#include "vk_initializers.h"

TextureViewer::~TextureViewer()
{
	pipeline->cleanup(*device);
}

void TextureViewer::initialize(VulkanTutorialExtension* engine)
{
	device = engine->getDevicePtr();
	createPipeline(engine);
}

void TextureViewer::createPipeline(VulkanTutorialExtension* engine)
{
	pipeline = std::make_shared<SimplePipelineEmptyInput>(
		engine->getSwapchainExtent(),
		"shaders/LightingPassvert.spv",
		"shaders/TextureViewerfrag.spv",
		SimplePipeline::RenderType::forward,
		engine->descriptorPool,
		1
	);
	pipeline->getDescriptorBuilder().addTexture(VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline->buildPipeline(engine, [renderPass = engine->getDefaultRenderPass()](VkGraphicsPipelineCreateInfo& pipelineCI) {
		pipelineCI.renderPass = renderPass;
		auto* depthStencil = const_cast<VkPipelineDepthStencilStateCreateInfo*>(pipelineCI.pDepthStencilState);
		depthStencil->depthTestEnable = VK_FALSE;
		depthStencil->depthWriteEnable = VK_FALSE;

		auto* rasterizer = const_cast<VkPipelineRasterizationStateCreateInfo*>(pipelineCI.pRasterizationState);
		rasterizer->cullMode = VK_CULL_MODE_FRONT_BIT;
		});

	addTexture(engine->getDefaultTexture2D(), "");
}

void TextureViewer::updateTextureIfNeeded(VulkanTutorialExtension* engine) 
{
	// 텍스처 인덱스가 변경된 경우에만 업데이트
	if (lastUpdatedIndex != targetTextureIndex && !textures.empty() && textures[targetTextureIndex].image) {
		VkImageView imageView = textures[targetTextureIndex].image->imageView;
		pipeline->updateTextureDescriptor(engine->getDevice(), 0, 0, imageView, engine->getDefaultTextureSampler());
		lastUpdatedIndex = targetTextureIndex;
	}
}
void TextureViewer::draw(VkCommandBuffer commandBuffer, VulkanTutorialExtension* engine)
{
	updateTextureIfNeeded(engine);

	VkExtent2D Res = engine->getSwapchainExtent();

	VkViewport viewport = vkb::initializers::viewport(static_cast<float>(Res.width), static_cast<float>(Res.height), 0.0f, 1.0f);
	VkRect2D scissor = vkb::initializers::rect2D(Res.width, Res.height, 0, 0);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline().pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getPipeline().layout, 0, 1, &pipeline->getDescriptorSets()[0], 0, nullptr);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	
	// Final composition
	// This is done by simply drawing a full screen quad
	// The fragment shader then combines the geometry attachments into the final image
	// Note: Also used for debug display if debugDisplayTarget > 0
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void TextureViewer::addTexture(const std::shared_ptr<CubeMap>& cubeMap, const std::string& name) {
	TextureInfo info = {};
	info.cubeMap = cubeMap;
	info.name = name.empty() ? "Texture " + std::to_string(textures.size()) : name;
	textures.push_back(info);
}

void TextureViewer::addTexture(const std::shared_ptr<AllocatedImage>& image, const std::string& name) {
	TextureInfo info = {};
	info.image = image;
	info.name = name.empty() ? "Texture " + std::to_string(textures.size()) : name;
	textures.push_back(info);
}

void TextureViewer::selectTexture(int index) {
	if (index >= 0 && index < textures.size()) {
		targetTextureIndex = index;
	}
}

void TextureViewer::selectTexture(const std::string& name) {
	for (int i = 0; i < textures.size(); i++) {
		if (textures[i].name == name) {
			targetTextureIndex = i;
			return;
		}
	}
}