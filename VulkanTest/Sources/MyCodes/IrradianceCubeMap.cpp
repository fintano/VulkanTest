#include "IrradianceCubeMap.h"
#include "vk_initializers.h"
#include "vk_descriptor.h"
#include "VulkanTools.h"
#include "VulkanTutorialExtension.h"
#include "GPUMarker.h"

#include <array>
#include <stb_image.h>

IrradianceCubeMap::IrradianceCubeMap(VkDevice inDevice, VkDescriptorPool inDescriptorPool, VkSampler inDefaultSampler) :
	device(inDevice),
	descriptorPool(inDescriptorPool),
	defaultSampler(inDefaultSampler)
{
}

void IrradianceCubeMap::initialize(VulkanTutorialExtension* engine)
{
	loadEquirectangular(engine, equirectangularPath);

	createCubeMap(engine);

	createRenderPass();

	createFrameBuffers();

	buildPipeline();

	writeDescriptor();
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

void IrradianceCubeMap::createCubeMap(VulkanTutorial* engine)
{
	AllocatedImage allocatedImage = 
	engine->createImage(Res.width, Res.height, 1, VK_SAMPLE_COUNT_1_BIT, HDRFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "IBLCubeMap", Faces, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

	image = allocatedImage.image;
	imageMemory = allocatedImage.imageMemory;

	imageViews.resize(Faces);
	for (int i = 0; i < Faces; i++)
	{
		imageViews[i] = engine->createImageView(image, HDRFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, i);
	}

	cubeImageView = engine->createImageViewCube(image, HDRFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
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
}

void IrradianceCubeMap::createFrameBuffers()
{
	frameBuffers.resize(Faces);
	for (int i = 0; i < Faces; i++)
	{
		VkFramebufferCreateInfo framebufferInfo = vkb::initializers::framebuffer_create_info();
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &imageViews[i];
		framebufferInfo.width = Res.width;
		framebufferInfo.height = Res.height;
		framebufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffers[i]));
	}
}

void IrradianceCubeMap::buildPipeline()
{
	// create layout
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	vk::desc::createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	layout = vk::desc::createDescriptorSetLayout(device, bindings);

	// push constant
	VkPushConstantRange matrixRange{};
	matrixRange.offset = 0;
	matrixRange.size = sizeof(GPUDrawPushConstants);
	matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::vector<VkDescriptorSetLayout> layouts = { layout };
	VkPipelineLayoutCreateInfo mesh_layout_info = vkb::initializers::pipeline_layout_create_info(layouts.size());
	mesh_layout_info.pSetLayouts = layouts.data();
	mesh_layout_info.pPushConstantRanges = &matrixRange;
	mesh_layout_info.pushConstantRangeCount = 1;

	VkPipelineLayout newLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &mesh_layout_info, nullptr, &newLayout));
	pipeline.layout = newLayout;

	// create pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vkb::initializers::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineViewportStateCreateInfo viewportState = vkb::initializers::pipeline_viewport_state_create_info(1, 1, 0);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vkb::initializers::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = vkb::initializers::pipeline_multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, 0);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vkb::initializers::pipeline_depth_stencil_state_create_info(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

	VkGraphicsPipelineCreateInfo pipelineCI = vkb::initializers::pipeline_create_info(newLayout, renderPass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;

	// Viewport
	VkViewport viewport = vkb::initializers::viewport((float)Res.width, (float)Res.height, 0.f, 1.f);
	VkRect2D scissor = vkb::initializers::rect2D(Res.width, Res.height, 0, 0);
	viewportState.pViewports = &viewport;
	viewportState.pScissors = &scissor;

	// ColorBlending
	std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = { vkb::initializers::pipeline_color_blend_attachment_state(0xf, VK_FALSE) };
	VkPipelineColorBlendStateCreateInfo colorBlendState = vkb::initializers::pipeline_color_blend_state_create_info(blendAttachmentStates.size(), blendAttachmentStates.data());
	pipelineCI.pColorBlendState = &colorBlendState;

	// Shaders
	VkShaderModule meshVertexShader = vks::tools::loadShader("shaders/IrradianceCubeMapvert.spv", device);
	VkShaderModule meshFragShader = vks::tools::loadShader("shaders/IrradianceCubeMapfrag.spv", device);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = meshVertexShader;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = meshFragShader;
	fragShaderStageInfo.pName = "main";

	std::array<VkPipelineShaderStageCreateInfo, 2 > shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();
	
	// vertices
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	VertexOnlyPos::getBindingDescriptions(bindingDescriptions);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	VertexOnlyPos::getAttributeDescriptions(attributeDescriptions);
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = vkb::initializers::pipeline_vertex_input_state_create_info();
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	pipelineCI.pVertexInputState = &vertexInputInfo;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline.pipeline));
}

void IrradianceCubeMap::writeDescriptor()
{
	VkDescriptorSetAllocateInfo allocInfo = vkb::initializers::descriptor_set_allocate_info(descriptorPool, &layout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));

	VkDescriptorImageInfo texDescriptor =
		vkb::initializers::descriptor_image_info(
			defaultSampler,
			equirectangularTexture.imageView,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
		vkb::initializers::write_descriptor_set(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texDescriptor),
	};

	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void IrradianceCubeMap::draw(VkCommandBuffer commandBuffer, const GPUMeshBuffers<VertexOnlyPos>& mesh)
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

	for (int i = 0; i < Faces; i++)
	{
		VkRenderPassBeginInfo renderPassInfo = vkb::initializers::render_pass_begin_info();
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffers[i];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = { Res.width,Res.height };

		std::array<VkClearValue, 1> clearValues{};
		clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		GPUMarker Marker(commandBuffer, "IrradianceCubeMap");
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptorSet, 0, nullptr);

		GPUDrawPushConstants pushConstants;
		pushConstants.model = captureProjection * captureViews[i];
		vkCmdPushConstants(commandBuffer, pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.Buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);

		// 드로우 커맨드 실행 (36개의 인덱스 = 12개 삼각형 = 6면의 큐브)
		vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffer);
	}
}

void IrradianceCubeMap::clear()
{
	equirectangularTexture.Destroy(device);

	vkDestroyPipeline(device, pipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
	vkDestroyDescriptorSetLayout(device, layout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);

	for (int i = 0; i < Faces; i++)
	{
		vkDestroyFramebuffer(device, frameBuffers[i], nullptr);
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
}
