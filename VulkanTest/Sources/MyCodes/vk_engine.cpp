#include "vk_engine.h"
#include "vk_initializers.h"
#include "VulkanTutorial.h"
#include "vk_descriptor.h"
#include "VulkanTools.h"

namespace vkinit = vkb::initializers;

void GLTFMetallic_Roughness::build_pipelines(VulkanTutorial* engine)
{
	VkPushConstantRange matrixRange{};
	matrixRange.offset = 0;
	matrixRange.size = sizeof(GPUDrawPushConstants);
	matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	DescriptorLayoutBuilder layoutBuilder;
	layoutBuilder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	layoutBuilder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layoutBuilder.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	materialLayout = layoutBuilder.build(engine->device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	VkDescriptorSetLayout layouts[] = { engine->gpuSceneDataDescriptorLayout, materialLayout };

	VkPipelineLayoutCreateInfo mesh_layout_info = vkinit::pipeline_layout_create_info();
	mesh_layout_info.setLayoutCount = 2;
	mesh_layout_info.pSetLayouts = layouts;
	mesh_layout_info.pPushConstantRanges = &matrixRange;
	mesh_layout_info.pushConstantRangeCount = 1;

	VkPipelineLayout newLayout;
	if (vkCreatePipelineLayout(engine->device, &mesh_layout_info, nullptr, &newLayout) != VK_SUCCESS)
	{
		std::runtime_error("failed to create pipeline layout!");
	}

	opaquePipeline.layout = newLayout;
	transparentPipeline.layout = newLayout;

	// build the stage-create-info for both vertex and fragment stages. This lets
	// the pipeline know the shader modules per stage
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vkinit::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = vkinit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = vkinit::pipeline_color_blend_attachment_state(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vkinit::pipeline_color_blend_state_create_info(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vkinit::pipeline_depth_stencil_state_create_info(VK_TRUE, VK_TRUE, VK_COMPARE_OP_GREATER_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = vkinit::pipeline_viewport_state_create_info(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = vkinit::pipeline_multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicState = vkinit::pipeline_dynamic_state_create_info(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCI = vkinit::pipeline_create_info(newLayout, engine->forward.renderPass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	VkShaderModule meshFragShader = engine->createShaderModule("../../shaders/mesh.frag.spv");
	////if (!vkutil::load_shader_module("../../shaders/mesh.frag.spv", engine->_device, &meshFragShader)) {
	////	fmt::println("Error when building the triangle fragment shader module");
	////}

	VkShaderModule meshVertexShader = engine->createShaderModule("../../shaders/mesh.vert.spv");
	////if (!vkutil::load_shader_module("../../shaders/mesh.vert.spv", engine->_device, &meshVertexShader)) {
	////	fmt::println("Error when building the triangle vertex shader module");
	////}

	shaderStages[0] = vkinit::shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, meshVertexShader);
	shaderStages[1] = vkinit::shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, meshFragShader);


	/*PipelineBuilder pipelineBuilder;
	pipelineBuilder.set_shaders(meshVertexShader, meshFragShader);
	pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.set_multisampling_none();
	pipelineBuilder.disable_blending();
	pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);*/ 
	// => VK_COMPARE_OP_GREATER_OR_EQUAL 이상하네.

	//render format
	//pipelineBuilder.set_color_attachment_format(engine->_drawImage.imageFormat);
	//pipelineBuilder.set_depth_format(engine->_depthImage.imageFormat);

	// use the triangle layout we created
	//pipelineBuilder._pipelineLayout = newLayout;

	// finally build the pipeline
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &opaquePipeline.pipeline));

	//opaquePipeline.pipeline = pipelineBuilder.build_pipeline(engine->_device);

	// create the transparent variant
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.blendEnable = VK_TRUE;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;  // 소스 색상 블렌딩 팩터
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // 대상 색상 블렌딩 팩터
	blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;  // 색상 블렌딩 연산
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // 소스 알파 블렌딩 팩터
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // 대상 알파 블렌딩 팩터
	blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;  // 알파 블렌딩 연산

	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = false;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = {};
	depthStencilState.back = {};
	depthStencilState.minDepthBounds = 0.f;
	depthStencilState.maxDepthBounds = 1.f;

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &transparentPipeline.pipeline));

	vkDestroyShaderModule(engine->device, meshFragShader, nullptr);
	vkDestroyShaderModule(engine->device, meshVertexShader, nullptr);
}

MaterialInstance GLTFMetallic_Roughness::write_material(VkDevice device, MaterialPass pass, const MaterialResources& resources, VulkanTutorial* engine)
{
	MaterialInstance matData;
	matData.passType = pass;
	if (pass == MaterialPass::Transparent) {
		matData.pipeline = &transparentPipeline;
	}
	else {
		matData.pipeline = &opaquePipeline;
	}

	engine->createMaterialDescriptorSets(materialLayout, matData.materialSet);

	//matData.materialSet = descriptorAllocator.allocate(device, materialLayout);


	//writer.clear();

	std::vector<VkWriteDescriptorSet> descriptorWrites;

	resources.dataBuffer.createWriteDescriptorSet

	writer.write_buffer(0, resources.dataBuffer, sizeof(MaterialConstants), resources.dataBufferOffset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.write_image(1, resources.colorImage.ImageView, resources.colorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.write_image(2, resources.metalRoughImage.ImageView, resources.metalRoughSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	//writer.update_set(device, matData.materialSet);
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	return matData;
}