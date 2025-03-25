#include "IrradianceCubeMap.h"
#include "vk_initializers.h"
#include "vk_descriptor.h"
#include "VulkanTools.h"
#include "VulkanTutorialExtension.h"
#include "GPUMarker.h"
#include "Cube.h"
#include "Quad.h"
#include "SimplePipeline.h"

#include <array>
#include <stb_image.h>

struct RoughnessBuffer {
	alignas(16) float roughness;
};

IrradianceCubeMap::IrradianceCubeMap(VkDevice inDevice, VkDescriptorPool inDescriptorPool) :
	device(inDevice),
	descriptorPool(inDescriptorPool)
{
}

void IrradianceCubeMap::initialize(VulkanTutorialExtension* engine)
{
	cube = std::make_shared<Cube<VertexOnlyPos>>();
	cube->createMesh(engine);

	quad = std::make_shared<Quad<VertexOnlyTex>>();
	quad->createMesh(engine);

	loadEquirectangular(engine, equirectangularPath);

	const int envMipLevels = static_cast<uint32_t>(std::floor(std::log2(max(Res.width, Res.height))));

	envCubeMap = createCubeImage(engine, envMipLevels, Res, HDRFormat, "IBLCubeMap");
	diffuseMap = createCubeImage(engine, 1, DiffuseMapRes, HDRFormat, "DiffuseMap");
	specularPrefilteredMap = createCubeImage(engine, maxMipLevels, SpecularMapRes, HDRFormat, "SpecularMap");
	specularBRDFLUT = create2DImage(engine, 1, IntegraionMapRes, VK_FORMAT_R16G16_SFLOAT, "BRDFIntegration");

	createSampler(engine->device);

	createRenderPass();

	createFrameBuffer(frameBuffers, 1, envCubeMap, Res);
	createFrameBuffer(diffuseFrameBuffers, 1, diffuseMap, DiffuseMapRes);
	createFrameBuffer(specularFrameBuffers, maxMipLevels, specularPrefilteredMap, SpecularMapRes);
	integrationFrameBuffers = createFrameBuffer2D(specularBRDFLUT, IntegraionMapRes, integrationRenderPass);

	buildPipeline(engine);

	VkCommandBuffer singleCommandBuffer = engine->beginSingleTimeCommands();
	draw(singleCommandBuffer, engine);
	engine->endSingleTimeCommands(singleCommandBuffer);
}

void IrradianceCubeMap::loadEquirectangular(VulkanTutorial* engine, const std::string& path)
{
	int texWidth, texHeight, texChannels;
	float* floatPixels = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
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
	samplerInfo.maxLod = 1;

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

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));

	// Integration 맵용 렌더패스 별도 생성
	colorAttachment.format = VK_FORMAT_R16G16_SFLOAT;

	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &integrationRenderPass));
}

void IrradianceCubeMap::buildPipeline(VulkanTutorialExtension* engine)
{
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
	envMapPipeline->buildPipeline(engine, [this](VkGraphicsPipelineCreateInfo& pipelineCI) {
		pipelineCI.renderPass = this->renderPass;
		auto* depthStencil = const_cast<VkPipelineDepthStencilStateCreateInfo*>(pipelineCI.pDepthStencilState);
		depthStencil->depthTestEnable = VK_FALSE;
		depthStencil->depthWriteEnable = VK_FALSE;
		});
	envMapPipeline->updateTextureDescriptor(device, 0, 0, equirectangularTexture.imageView, defaultSampler);

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
	diffuseMapPipeline->buildPipeline(engine, [this](VkGraphicsPipelineCreateInfo& pipelineCI) {
		pipelineCI.renderPass = this->renderPass;
		auto* depthStencil = const_cast<VkPipelineDepthStencilStateCreateInfo*>(pipelineCI.pDepthStencilState);
		depthStencil->depthTestEnable = VK_FALSE;
		depthStencil->depthWriteEnable = VK_FALSE;
		});
	diffuseMapPipeline->updateTextureDescriptor(device, 0, 0, envCubeMap.cubeImageView, defaultSampler);

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
	specularMapPipeline->buildPipeline(engine, [this](VkGraphicsPipelineCreateInfo& pipelineCI) {
		pipelineCI.renderPass = this->renderPass;
		auto* depthStencil = const_cast<VkPipelineDepthStencilStateCreateInfo*>(pipelineCI.pDepthStencilState);
		depthStencil->depthTestEnable = VK_FALSE;
		depthStencil->depthWriteEnable = VK_FALSE;
		});
	specularMapPipeline->updateTextureDescriptor(device, 0, 0, envCubeMap.cubeImageView, defaultSampler);

	// BRDF LUT 파이프라인
	integrationMapPipeline = std::make_shared<SimplePipelineTexOnly>(
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
	for (int i = 0; i < Faces; i++)
	{
		VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffers[0][i];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = { Res.width,Res.height };

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		GPUMarker Marker(commandBuffer, "IrradianceCubeMap");
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, envMapPipeline->getPipeline().pipeline);

		VkViewport viewport = vkb::initializers::viewport(static_cast<float>(Res.width), static_cast<float>(Res.height), 0.0f, 1.0f);
		VkRect2D scissor = vkb::initializers::rect2D(Res.width, Res.height, 0, 0);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, envMapPipeline->getPipeline().layout, 0, 1, &envMapPipeline->getDescriptorSets()[0], 0, nullptr);

		GPUDrawPushConstants pushConstants;
		pushConstants.model = captureProjection * captureViews[i];
		vkCmdPushConstants(commandBuffer, envMapPipeline->getPipeline().layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->mesh.vertexBuffer.Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, cube->mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

		// 드로우 커맨드 실행 (36개의 인덱스 = 12개 삼각형 = 6면의 큐브)
		vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
	}

	/** Generate mipmaps of HDR cubeamp */
	const int mipLevels = static_cast<uint32_t>(std::floor(std::log2(max(Res.width, Res.height))));
	engine->generateMipmaps(commandBuffer, envCubeMap.image, HDRFormat, Res.width, Res.height, mipLevels, 6);

	/** HDR Cubemap to diffuse environment map */

	for (int i = 0; i < Faces; i++)
	{
		VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = diffuseFrameBuffers[0][i];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = { DiffuseMapRes.width,DiffuseMapRes.height };

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		GPUMarker Marker(commandBuffer, "DiffuseMap");
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, diffuseMapPipeline->getPipeline().pipeline);

		VkViewport viewport = vkb::initializers::viewport(static_cast<float>(DiffuseMapRes.width), static_cast<float>(DiffuseMapRes.height), 0.0f, 1.0f);
		VkRect2D scissor = vkb::initializers::rect2D(DiffuseMapRes.width, DiffuseMapRes.height, 0, 0);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, diffuseMapPipeline->getPipeline().layout, 0, 1, &diffuseMapPipeline->getDescriptorSets()[0], 0, nullptr);

		GPUDrawPushConstants pushConstants;
		pushConstants.model = captureProjection * captureViews[i];
		vkCmdPushConstants(commandBuffer, diffuseMapPipeline->getPipeline().layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->mesh.vertexBuffer.Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, cube->mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
	}

	/** HDR Cubemap to specular environment map */

	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = SpecularMapRes.width * std::pow(0.5, mip);
		unsigned int mipHeight = SpecularMapRes.height * std::pow(0.5, mip);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		for (unsigned int i = 0; i < 6; ++i)
		{
			VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = specularFrameBuffers[mip][i];
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = { mipWidth,mipHeight };

			std::array<VkClearValue, 1> clearValues{};
			clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			GPUMarker Marker(commandBuffer, "SpecularMap");
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, specularMapPipeline->getPipeline().pipeline);

			VkViewport viewport = vkb::initializers::viewport(static_cast<float>(mipWidth), static_cast<float>(mipHeight), 0.0f, 1.0f);
			VkRect2D scissor = vkb::initializers::rect2D(mipWidth, mipHeight, 0, 0);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, specularMapPipeline->getPipeline().layout, 0, 1, &specularMapPipeline->getDescriptorSets()[0], 0, nullptr);

			GPUDrawPushConstants pushConstants;
			pushConstants.model = captureProjection * captureViews[i];
			vkCmdPushConstants(commandBuffer, specularMapPipeline->getPipeline().layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
			vkCmdPushConstants(commandBuffer, specularMapPipeline->getPipeline().layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(GPUDrawPushConstants), sizeof(float), &roughness);

			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &cube->mesh.vertexBuffer.Buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, cube->mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
			vkCmdEndRenderPass(commandBuffer);
		}
	}

	/** Specular BRDF Integration map*/

	VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
	renderPassInfo.renderPass = integrationRenderPass;
	renderPassInfo.framebuffer = integrationFrameBuffers;
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = IntegraionMapRes;

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	GPUMarker Marker(commandBuffer, "IntegrationMap");
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, integrationMapPipeline->getPipeline().pipeline);

	VkViewport viewport = vkb::initializers::viewport(static_cast<float>(IntegraionMapRes.width), static_cast<float>(IntegraionMapRes.height), 0.0f, 1.0f);
	VkRect2D scissor = vkb::initializers::rect2D(IntegraionMapRes.width, IntegraionMapRes.height, 0, 0);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &quad->mesh.vertexBuffer.Buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, quad->mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
	vkCmdEndRenderPass(commandBuffer);
}

void IrradianceCubeMap::clear()
{
	cube->cleanUp(device);
	quad->cleanUp(device);
	equirectangularTexture.Destroy(device);

	envMapPipeline->cleanup(device);
	diffuseMapPipeline->cleanup(device);
	specularMapPipeline->cleanup(device);
	integrationMapPipeline->cleanup(device);

	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyRenderPass(device, integrationRenderPass, nullptr);

	auto destroyCubeFrameBuffer = [&](CubeFrameBuffer& thisFrameBuffer) {
		for (int i = 0; i < thisFrameBuffer.size(); i++)
		{
			for (int j = 0; j < thisFrameBuffer[i].size(); j++)
			{
				vkDestroyFramebuffer(device, thisFrameBuffer[i][j], nullptr);
			}
		};
	};

	destroyCubeFrameBuffer(frameBuffers);
	destroyCubeFrameBuffer(diffuseFrameBuffers);
	destroyCubeFrameBuffer(specularFrameBuffers);
	vkDestroyFramebuffer(device, integrationFrameBuffers, nullptr);

	vkDestroySampler(device, defaultSampler, nullptr);

	envCubeMap.Destroy(device);
	diffuseMap.Destroy(device);
	specularPrefilteredMap.Destroy(device);
	specularBRDFLUT.Destroy(device);
}

CubeMap IrradianceCubeMap::createCubeImage(VulkanTutorial* engine, uint32_t mipLevels, VkExtent2D extent, VkFormat format, const std::string& debugName)
{
	CubeMap cubeMap;

	AllocatedImage allocatedImage =
		engine->createImage(extent.width, extent.height, mipLevels, VK_SAMPLE_COUNT_1_BIT, format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			debugName.c_str(), Faces, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

	cubeMap.image = allocatedImage.image;
	cubeMap.imageMemory = allocatedImage.imageMemory;

	cubeMap.imageViews.resize(mipLevels);
	for (int i = 0; i < mipLevels; i++)
	{
		cubeMap.imageViews[i].resize(Faces);
		for (int j = 0; j < Faces; j++) {
			cubeMap.imageViews[i][j] = engine->createImageView(cubeMap.image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1, j, i);
		}
	}

	cubeMap.cubeImageView = engine->createImageViewCube(cubeMap.image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	return cubeMap;
}

AllocatedImage IrradianceCubeMap::create2DImage(VulkanTutorial* engine, uint32_t mipLevels, VkExtent2D extent, VkFormat format, const std::string& debugName)
{
	AllocatedImage allocatedImage =
		engine->createImage(extent.width, extent.height, mipLevels, VK_SAMPLE_COUNT_1_BIT, format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			debugName.c_str(), 1);

	allocatedImage.imageView = engine->createImageView(allocatedImage.image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

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

			VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffers[i][j]));
		}
	}
}

VkFramebuffer IrradianceCubeMap::createFrameBuffer2D(const AllocatedImage& image, VkExtent2D extent, VkRenderPass renderPass)
{
	VkRenderPass useRenderPass = (renderPass != VK_NULL_HANDLE) ? renderPass : this->renderPass;

	VkFramebuffer frameBuffer;

	VkFramebufferCreateInfo framebufferInfo = vkb::initializers::framebuffer_create_info();
	framebufferInfo.renderPass = useRenderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &image.imageView;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffer));

	return frameBuffer;
}