#include "IrradianceCubeMap.h"
#include "vk_initializers.h"
#include "vk_descriptor.h"
#include "VulkanTools.h"
#include "VulkanTutorialExtension.h"
#include "GPUMarker.h"
#include "Cube.h"
#include "Quad.h"
#include "SimplePipeline.h"
#include "vk_resource_utils.h"
#include "TextureViewer.h"

#include <array>

struct RoughnessBuffer {
	alignas(16) float roughness;
};

IrradianceCubeMap::IrradianceCubeMap(DevicePtr inDevice, VkDescriptorPool inDescriptorPool) :
	device(inDevice),
	descriptorPool(inDescriptorPool),
	envMipLevels(static_cast<uint32_t>(std::floor(std::log2(max(Res.width, Res.height)))))
{
}

void IrradianceCubeMap::initialize(VulkanTutorialExtension* engine)
{
	cube = std::make_shared<Cube<VertexOnlyPos>>();
	cube->createMesh(engine);

	quad = std::make_shared<Quad<VertexOnlyTex>>();
	quad->createMesh(engine);

	loadEquirectangular(engine, equirectangularPath);

	envCubeMap = createCubeImage(engine, envMipLevels, Res, HDRFormat, "IBLCubeMap");
	diffuseMap = createCubeImage(engine, 1, DiffuseMapRes, HDRFormat, "DiffuseMap");
	specularPrefilteredMap = createCubeImage(engine, maxMipLevels, SpecularMapRes, HDRFormat, "SpecularMap");
	specularBRDFLUT = create2DImage(engine, 1, IntegraionMapRes, VK_FORMAT_R16G16_SFLOAT, "BRDFIntegration");

	createSampler(engine->getDevice());

	createRenderPass();

	createFrameBuffer(frameBuffers, envMipLevels, *envCubeMap, Res, envCubeRenderPass);
	createFrameBuffer(diffuseFrameBuffers, 1, *diffuseMap, DiffuseMapRes);
	createFrameBuffer(specularFrameBuffers, maxMipLevels, *specularPrefilteredMap, SpecularMapRes);
	integrationFrameBuffers = createFrameBuffer2D(specularBRDFLUT, IntegraionMapRes, integrationRenderPass);

	buildPipeline(engine);

	VkCommandBuffer singleCommandBuffer = engine->beginSingleTimeCommands();
	draw(singleCommandBuffer, engine);
	engine->endSingleTimeCommands(singleCommandBuffer);

	// for debugging
	std::shared_ptr<TextureViewer> textureViewer = engine->getTextureViewer();
	textureViewer->addTexture(envCubeMap, "EnvironmentMap");
	textureViewer->addTexture(diffuseMap, "DiffuseMap");
	textureViewer->addTexture(specularPrefilteredMap, "SpecularPrefilteredMap");
	textureViewer->addTexture(specularBRDFLUT, "SpecularBRDFLUT");
}

void IrradianceCubeMap::loadEquirectangular(VulkanTutorial* engine, const std::string& path)
{
	int texWidth, texHeight, texChannels;
	float* floatPixels = Utils::loadImagef(path.c_str(), &texWidth, &texHeight, &texChannels, Utils::STBI_rgb_alpha);
	if (!floatPixels)
	{
		throw std::runtime_error("failed to load texture image!");
	}

	equirectangularTexture = engine->createTexture2D((stbi_uc*)floatPixels, { (uint32_t)texWidth ,(uint32_t)texHeight, 1 }, HDRFormat, VK_IMAGE_USAGE_SAMPLED_BIT, "HDR");
}

void IrradianceCubeMap::createSampler(VkDevice device)
{
	VkSamplerCreateInfo samplerInfo = vkb::initializers::sampler_create_info();
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // we will look at this on shader map chapter.
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.f;
	samplerInfo.minLod = 0;
	samplerInfo.maxLod = envMipLevels;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &defaultSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void IrradianceCubeMap::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = HDRFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	
	dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // 제약 없음
	dependency.srcAccessMask = 0; // 특별한 이전 접근이 없다면 0

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // 이 때 Attachment의 레이아웃 암묵적 변환 (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments =&colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK_RESULT(vkCreateRenderPass(*device, &renderPassInfo, nullptr, &renderPass));

	// env 맵용 렌더 패스 
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	VK_CHECK_RESULT(vkCreateRenderPass(*device, &renderPassInfo, nullptr, &envCubeRenderPass));

	// Integration 맵용 렌더패스 
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	colorAttachment.format = VK_FORMAT_R16G16_SFLOAT;

	VK_CHECK_RESULT(vkCreateRenderPass(*device, &renderPassInfo, nullptr, &integrationRenderPass));
}

void IrradianceCubeMap::buildPipeline(VulkanTutorialExtension* engine)
{
	auto disableDepthTestAndWrite = [this](VkGraphicsPipelineCreateInfo& pipelineCI) {
		pipelineCI.renderPass = this->renderPass;
		auto* depthStencil = const_cast<VkPipelineDepthStencilStateCreateInfo*>(pipelineCI.pDepthStencilState);
		depthStencil->depthTestEnable = VK_FALSE;
		depthStencil->depthWriteEnable = VK_FALSE;
		};

	// 환경 맵 파이프라인
	envMapPipeline = std::make_shared<SimplePipelinePosOnly>(
		Res,
		"shaders/IrradianceCubeMapvert.spv",
		"shaders/IrradianceCubeMapfrag.spv",
		SimplePipeline::RenderType::forward,
		descriptorPool,
		1
	);
	envMapPipeline->getDescriptorBuilder().addTexture(VK_SHADER_STAGE_FRAGMENT_BIT);
	envMapPipeline->addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GPUDrawPushConstants), 0);
	envMapPipeline->buildPipeline(engine, disableDepthTestAndWrite);
	envMapPipeline->updateTextureDescriptor(*device, 0, 0, equirectangularTexture->imageView, defaultSampler);

	// 디퓨즈 맵 파이프라인
	diffuseMapPipeline = std::make_shared<SimplePipelinePosOnly>(
		DiffuseMapRes,
		"shaders/IrradianceCubeMapvert.spv",
		"shaders/DiffuseMapfrag.spv",
		SimplePipeline::RenderType::forward,
		descriptorPool,
		1
	);
	diffuseMapPipeline->getDescriptorBuilder().addTexture(VK_SHADER_STAGE_FRAGMENT_BIT);
	diffuseMapPipeline->addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GPUDrawPushConstants), 0);
	diffuseMapPipeline->buildPipeline(engine, disableDepthTestAndWrite);
	diffuseMapPipeline->updateTextureDescriptor(*device, 0, 0, envCubeMap->image->imageView, defaultSampler);

	// 스페큘러 맵 파이프라인
	specularMapPipeline = std::make_shared<SimplePipelinePosOnly>(
		SpecularMapRes,
		"shaders/IrradianceCubeMapvert.spv",
		"shaders/SpecularMapfrag.spv",
		SimplePipeline::RenderType::forward,
		descriptorPool,
		1
	);
	specularMapPipeline->getDescriptorBuilder().addTexture(VK_SHADER_STAGE_FRAGMENT_BIT);
	specularMapPipeline->addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GPUDrawPushConstants), 0);
	specularMapPipeline->addPushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(float), sizeof(GPUDrawPushConstants));
	specularMapPipeline->buildPipeline(engine, disableDepthTestAndWrite);
	specularMapPipeline->updateTextureDescriptor(*device, 0, 0, envCubeMap->image->imageView, defaultSampler);

	// BRDF LUT 파이프라인
	integrationMapPipeline = std::make_shared<SimplePipelinePosTex>(
		SpecularMapRes,
		"shaders/BRDFIntegrationMapvert.spv",
		"shaders/BRDFIntegrationMapfrag.spv",
		SimplePipeline::RenderType::forward,
		descriptorPool,
		1
	);
	integrationMapPipeline->buildPipeline(engine, [this](VkGraphicsPipelineCreateInfo& pipelineCI) {
		pipelineCI.renderPass = this->integrationRenderPass;
		auto* rasterState = const_cast<VkPipelineRasterizationStateCreateInfo*>(pipelineCI.pRasterizationState);
		rasterState->cullMode = VK_CULL_MODE_NONE; // 컬링 비활성화
		auto* depthStencil = const_cast<VkPipelineDepthStencilStateCreateInfo*>(pipelineCI.pDepthStencilState);
		depthStencil->depthTestEnable = VK_FALSE;
		depthStencil->depthWriteEnable = VK_FALSE;
		auto* colorBlend = const_cast<VkPipelineColorBlendStateCreateInfo*>(pipelineCI.pColorBlendState);
		auto* attachments = const_cast<VkPipelineColorBlendAttachmentState*>(colorBlend->pAttachments);
		attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
		attachments[0].blendEnable = VK_FALSE;
		});
}

void IrradianceCubeMap::draw(VkCommandBuffer commandBuffer, VulkanTutorialExtension* engine)
{
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	/** HDR equirectangularMap to HDR cubemap */
	{
		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
		VkViewport viewport = vkb::initializers::viewport(static_cast<float>(Res.width), static_cast<float>(Res.height), 0.0f, 1.0f);
		VkRect2D scissor = vkb::initializers::rect2D(Res.width, Res.height, 0, 0);
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, envMapPipeline->getPipeline().pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, envMapPipeline->getPipeline().layout, 0, 1, &envMapPipeline->getDescriptorSets()[0], 0, nullptr);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->mesh.vertexBuffer.Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, cube->mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

		GPUMarker Marker(commandBuffer, "IrradianceCubeMap");

		for (int i = 0; i < Faces; i++) {
			VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
			renderPassInfo.renderPass = envCubeRenderPass;
			renderPassInfo.framebuffer = frameBuffers[0][i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { Res.width, Res.height };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			GPUDrawPushConstants pushConstants;
			pushConstants.model = captureProjection * captureViews[i];
			vkCmdPushConstants(commandBuffer, envMapPipeline->getPipeline().layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

			vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
			vkCmdEndRenderPass(commandBuffer);
		}
	}

	/** Generate mipmaps of HDR cubemap */
	const int mipLevels = static_cast<uint32_t>(std::floor(std::log2(max(Res.width, Res.height))));
	engine->transitionImageLayout(commandBuffer, envCubeMap->image->image, HDRFormat,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		mipLevels - 1, 6, 1); // levelCount=mipLevels-1, layerCount=6, baseMipLevel=1

	engine->generateMipmaps(commandBuffer, envCubeMap->image->image, HDRFormat, Res.width, Res.height, mipLevels, 6);

	/** HDR Cubemap to diffuse environment map */
	{
		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
		VkViewport viewport = vkb::initializers::viewport(static_cast<float>(DiffuseMapRes.width), static_cast<float>(DiffuseMapRes.height), 0.0f, 1.0f);
		VkRect2D scissor = vkb::initializers::rect2D(DiffuseMapRes.width, DiffuseMapRes.height, 0, 0);
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, diffuseMapPipeline->getPipeline().pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, diffuseMapPipeline->getPipeline().layout, 0, 1, &diffuseMapPipeline->getDescriptorSets()[0], 0, nullptr);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->mesh.vertexBuffer.Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, cube->mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

		GPUMarker Marker(commandBuffer, "DiffuseMap");

		for (int i = 0; i < Faces; i++) {
			VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = diffuseFrameBuffers[0][i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = { DiffuseMapRes.width, DiffuseMapRes.height };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			GPUDrawPushConstants pushConstants;
			pushConstants.model = captureProjection * captureViews[i];
			vkCmdPushConstants(commandBuffer, diffuseMapPipeline->getPipeline().layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

			vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
			vkCmdEndRenderPass(commandBuffer);
		}
	}

	/** HDR Cubemap to specular environment map */
	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->mesh.vertexBuffer.Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, cube->mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, specularMapPipeline->getPipeline().pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, specularMapPipeline->getPipeline().layout, 0, 1, &specularMapPipeline->getDescriptorSets()[0], 0, nullptr);

		GPUMarker Marker(commandBuffer, "SpecularMap");

		for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
			// resize framebuffer according to mip-level size.
			unsigned int mipWidth = SpecularMapRes.width * std::pow(0.5, mip);
			unsigned int mipHeight = SpecularMapRes.height * std::pow(0.5, mip);

			VkViewport viewport = vkb::initializers::viewport(static_cast<float>(mipWidth), static_cast<float>(mipHeight), 0.0f, 1.0f);
			VkRect2D scissor = vkb::initializers::rect2D(mipWidth, mipHeight, 0, 0);

			float roughness = (float)mip / (float)(maxMipLevels - 1);

			for (unsigned int i = 0; i < 6; ++i) {
				std::array<VkClearValue, 1> clearValues{};
				clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };

				VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
				renderPassInfo.renderPass = renderPass;
				renderPassInfo.framebuffer = specularFrameBuffers[mip][i];
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = { mipWidth, mipHeight };
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
				renderPassInfo.pClearValues = clearValues.data();

				vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				GPUDrawPushConstants pushConstants;
				pushConstants.model = captureProjection * captureViews[i];
				vkCmdPushConstants(commandBuffer, specularMapPipeline->getPipeline().layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
				vkCmdPushConstants(commandBuffer, specularMapPipeline->getPipeline().layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GPUDrawPushConstants), sizeof(float), &roughness);

				vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
				vkCmdEndRenderPass(commandBuffer);
			}
		}
	}

	/** Specular BRDF Integration map */
	{
		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };

		VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
		renderPassInfo.renderPass = integrationRenderPass;
		renderPassInfo.framebuffer = integrationFrameBuffers;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = IntegraionMapRes;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		VkViewport viewport = vkb::initializers::viewport(static_cast<float>(IntegraionMapRes.width), static_cast<float>(IntegraionMapRes.height), 0.0f, 1.0f);
		VkRect2D scissor = vkb::initializers::rect2D(IntegraionMapRes.width, IntegraionMapRes.height, 0, 0);
		VkDeviceSize offsets[] = { 0 };

		GPUMarker Marker(commandBuffer, "IntegrationMap");

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, integrationMapPipeline->getPipeline().pipeline);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &quad->mesh.vertexBuffer.Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, quad->mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
	}
}

void IrradianceCubeMap::clear()
{
	cube->cleanUp(*device);
	quad->cleanUp(*device);

	envMapPipeline->cleanup(*device);
	diffuseMapPipeline->cleanup(*device);
	specularMapPipeline->cleanup(*device);
	integrationMapPipeline->cleanup(*device);

	vkDestroyRenderPass(*device, renderPass, nullptr);
	vkDestroyRenderPass(*device, envCubeRenderPass, nullptr);
	vkDestroyRenderPass(*device, integrationRenderPass, nullptr);

	auto destroyCubeFrameBuffer = [&](CubeFrameBuffer& thisFrameBuffer) {
		for (int i = 0; i < thisFrameBuffer.size(); i++)
		{
			for (int j = 0; j < thisFrameBuffer[i].size(); j++)
			{
				vkDestroyFramebuffer(*device, thisFrameBuffer[i][j], nullptr);
			}
		};
	};

	destroyCubeFrameBuffer(frameBuffers);
	destroyCubeFrameBuffer(diffuseFrameBuffers);
	destroyCubeFrameBuffer(specularFrameBuffers);
	vkDestroyFramebuffer(*device, integrationFrameBuffers, nullptr);

	vkDestroySampler(*device, defaultSampler, nullptr);
}

std::shared_ptr<CubeMap> IrradianceCubeMap::createCubeImage(VulkanTutorial* engine, uint32_t mipLevels, VkExtent2D extent, VkFormat format, const std::string& debugName)
{
	std::shared_ptr<CubeMap> cubeMap = std::make_shared<CubeMap>(engine->getDevicePtr());

	cubeMap->image =
		engine->createImage(extent.width, extent.height, mipLevels, VK_SAMPLE_COUNT_1_BIT, format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			debugName.c_str(), 
			Faces, 
			VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

	cubeMap->imageViews.resize(mipLevels);
	for (int i = 0; i < mipLevels; i++)
	{
		cubeMap->imageViews[i].resize(Faces);
		for (int j = 0; j < Faces; j++) {
			cubeMap->imageViews[i][j] = engine->createImageView(cubeMap->image->image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1, j, i);
		}
	}

	cubeMap->image->imageView = engine->createImageViewCube(cubeMap->image->image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	return cubeMap;
}

std::shared_ptr<AllocatedImage> IrradianceCubeMap::create2DImage(VulkanTutorial* engine, uint32_t mipLevels, VkExtent2D extent, VkFormat format, const std::string& debugName)
{
	std::shared_ptr<AllocatedImage> allocatedImage =
		engine->createImage(extent.width, extent.height, mipLevels, VK_SAMPLE_COUNT_1_BIT, format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			debugName.c_str(), 1);

	allocatedImage->imageView = engine->createImageView(allocatedImage->image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	return allocatedImage;
}

void IrradianceCubeMap::createFrameBuffer(CubeFrameBuffer& frameBuffers, uint32_t mipLevels, const CubeMap& cubeMap, VkExtent2D extent, VkRenderPass renderPass)
{
	VkRenderPass useRenderPass = (renderPass != VK_NULL_HANDLE) ? renderPass : this->renderPass;

	frameBuffers.resize(cubeMap.imageViews.size());
	for (int i = 0; i < cubeMap.imageViews.size(); i++)
	{
		// 현재 mip 레벨에 맞게 크기 계산
		uint32_t mipWidth = extent.width * std::pow(0.5, i);
		uint32_t mipHeight = extent.height * std::pow(0.5, i);

		frameBuffers[i].resize(cubeMap.imageViews[i].size());
		for (int j = 0; j < cubeMap.imageViews[i].size(); j++)
		{
			VkFramebufferCreateInfo framebufferInfo = vkb::initializers::framebuffer_create_info();
			framebufferInfo.renderPass = useRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &cubeMap.imageViews[i][j];
			framebufferInfo.width = mipWidth;
			framebufferInfo.height = mipHeight;
			framebufferInfo.layers = 1;

			VK_CHECK_RESULT(vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &frameBuffers[i][j]));
		}
	}
}

VkFramebuffer IrradianceCubeMap::createFrameBuffer2D(const std::shared_ptr<AllocatedImage>& image, VkExtent2D extent, VkRenderPass renderPass)
{
	VkRenderPass useRenderPass = (renderPass != VK_NULL_HANDLE) ? renderPass : this->renderPass;

	VkFramebuffer frameBuffer;

	VkFramebufferCreateInfo framebufferInfo = vkb::initializers::framebuffer_create_info();
	framebufferInfo.renderPass = useRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &image->imageView;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	VK_CHECK_RESULT(vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &frameBuffer));

	return frameBuffer;
}