#include "IrradianceCubeMap.h"
#include "vk_initializers.h"
#include "vk_descriptor.h"
#include "VulkanTools.h"

#include <array>

IrradianceCubeMap::IrradianceCubeMap(const AllocatedImage& inEquirectangular) : equirectangular(inEquirectangular)
{
}

IrradianceCubeMap::~IrradianceCubeMap()
{
}

void IrradianceCubeMap::createRenderPass(VkDevice device)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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

void IrradianceCubeMap::buildPipeline(VkDevice device, VkDescriptorSetLayout globalLayout)
{
	// create layout
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	vk::desc::createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	layout = vk::desc::createDescriptorSetLayout(device, bindings);

	std::array<VkDescriptorSetLayout, 2> layouts = { globalLayout, layout };
	VkPipelineLayoutCreateInfo mesh_layout_info = vkb::initializers::pipeline_layout_create_info(layouts.size());
	mesh_layout_info.pSetLayouts = layouts.data();

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
	VkViewport viewport = vkb::initializers::viewport((float)faceRes.width, (float)faceRes.height, 0.f, 1.f);
	VkRect2D scissor = vkb::initializers::rect2D(faceRes.width, faceRes.height, 0, 0);
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

	VkPipelineVertexInputStateCreateInfo emptyInputInfo = vkb::initializers::pipeline_vertex_input_state_create_info();
	pipelineCI.pVertexInputState = &emptyInputInfo;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline.pipeline));
}

void IrradianceCubeMap::clear()
{

}
