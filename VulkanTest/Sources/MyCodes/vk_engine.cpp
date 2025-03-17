#include "vk_engine.h"
#include "vk_initializers.h"
#include "vk_descriptor.h"
#include "VulkanTools.h"
#include "VulkanTutorialExtension.h"

namespace vkinit = vkb::initializers;

void GLTFMetallic_Roughness::build_pipelines(VulkanTutorialExtension* engine)
{
	VkExtent2D swapchainExtent = engine->getSwapchainExtent();

	//VkPushConstantRange matrixRange{};
	//matrixRange.offset = 0;
	//matrixRange.size = sizeof(GPUDrawPushConstants);
	//matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	vk::desc::createDescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	vk::desc::createDescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	vk::desc::createDescriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, bindings);

	vk::desc::createDescriptorSetLayout(engine->device, bindings, materialLayout);

	//DescriptorLayoutBuilder layoutBuilder;
	//layoutBuilder.add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	//layoutBuilder.add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	//layoutBuilder.add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	//materialLayout = layoutBuilder.build(engine->_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	//VkDescriptorSetLayout layouts[] = { engine->descriptorSetLayoutPointLights /*engine->_gpuSceneDataDescriptorLayout*/,
	//	materialLayout };

	std::array<VkDescriptorSetLayout, 2> layouts = { engine->descriptorSetLayoutPointLights , materialLayout };

	VkPipelineLayoutCreateInfo mesh_layout_info = vkinit::pipeline_layout_create_info(layouts.size());
	mesh_layout_info.pSetLayouts = layouts.data();
	//mesh_layout_info.pPushConstantRanges = &matrixRange;
	//mesh_layout_info.pushConstantRangeCount = 1;

	VkPipelineLayout newLayout;
	VK_CHECK_RESULT(vkCreatePipelineLayout(engine->device, &mesh_layout_info, nullptr, &newLayout));

	opaquePipeline.layout = newLayout;
	transparentPipeline.layout = newLayout;

	// Pipelines
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vkinit::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineViewportStateCreateInfo viewportState = vkinit::pipeline_viewport_state_create_info(1, 1, 0);
	//std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR }; // ³»°¡ ¾È½è´ø ¿É¼Ç
	//VkPipelineDynamicStateCreateInfo dynamicState = vkinit::pipeline_dynamic_state_create_info(dynamicStateEnables); // ³»°¡ ¾È½è´ø ¿É¼Ç
	VkPipelineRasterizationStateCreateInfo rasterizationState = vkinit::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = vkinit::pipeline_multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, 0);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = vkinit::pipeline_depth_stencil_state_create_info(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	
	VkGraphicsPipelineCreateInfo pipelineCI = vkinit::pipeline_create_info(newLayout, engine->forward.renderPass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	//pipelineCI.pDynamicState = &dynamicState;

	// Viewport
	VkViewport viewport = vkinit::viewport((float)swapchainExtent.width, (float)swapchainExtent.height, 0.f, 1.f);
	VkRect2D scissor = vkinit::rect2D(swapchainExtent.width, swapchainExtent.height, 0, 0);
	viewportState.pViewports = &viewport;
	viewportState.pScissors = &scissor;

	// ColorBlending	
	VkPipelineColorBlendAttachmentState blendAttachmentState = vkinit::pipeline_color_blend_attachment_state(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = vkinit::pipeline_color_blend_state_create_info(1, &blendAttachmentState);
	pipelineCI.pColorBlendState = &colorBlendState;


	VkShaderModule meshVertexShader = vks::tools::loadShader("shaders/shadervert.spv", engine->device);
	VkShaderModule meshFragShader = vks::tools::loadShader("shaders/ForwardPassfrag.spv", engine->device);

	// Shaders
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

	std::array<VkPipelineShaderStageCreateInfo,2 > shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	// VertexInputState	
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	Vertex::getBindingDescriptions(bindingDescriptions);

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	Vertex::getAttributeDescriptions(attributeDescriptions);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = vkinit::pipeline_vertex_input_state_create_info();
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	pipelineCI.pVertexInputState = &vertexInputInfo;


	for (size_t i = 0; i < bindings.size(); i++) {
		printf("Binding %zu: descriptorType=%d, stageFlags=%u\n",
			bindings[i].binding, bindings[i].descriptorType, bindings[i].stageFlags);
	}

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &opaquePipeline.pipeline));

	// build the stage-create-info for both vertex and fragment stages. This lets
	// the pipeline know the shader modules per stage
	//PipelineBuilder pipelineBuilder;
	//pipelineBuilder.set_shaders(meshVertexShader, meshFragShader);
	//pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
	//pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	//pipelineBuilder.set_multisampling_none();
	//pipelineBuilder.disable_blending();
	//pipelineBuilder.enable_depthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

	//render format
	//pipelineBuilder.set_color_attachment_format(engine->_drawImage.imageFormat);
	//pipelineBuilder.set_depth_format(engine->_depthImage.imageFormat);

	// use the triangle layout we created
	//pipelineBuilder._pipelineLayout = newLayout;

	// finally build the pipeline
	//opaquePipeline.pipeline = pipelineBuilder.build_pipeline(engine->_device);

	// ColorBlending
	{
		VkPipelineColorBlendAttachmentState blendAttachmentState = vkinit::pipeline_color_blend_attachment_state(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_TRUE);
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;  // ¼Ò½º »ö»ó ºí·»µù ÆÑÅÍ
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // ´ë»ó »ö»ó ºí·»µù ÆÑÅÍ
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;  // »ö»ó ºí·»µù ¿¬»ê
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // ¼Ò½º ¾ËÆÄ ºí·»µù ÆÑÅÍ
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // ´ë»ó ¾ËÆÄ ºí·»µù ÆÑÅÍ
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;  // ¾ËÆÄ ºí·»µù ¿¬»ê
		
		VkPipelineColorBlendStateCreateInfo colorBlendState = vkinit::pipeline_color_blend_state_create_info(1, &blendAttachmentState);
		pipelineCI.pColorBlendState = &colorBlendState;
	}

	// create the transparent variant
	//pipelineBuilder.enable_blending_additive();

	//pipelineBuilder.enable_depthtest(false, VK_COMPARE_OP_GREATER_OR_EQUAL);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &transparentPipeline.pipeline));

	//transparentPipeline.pipeline = pipelineBuilder.build_pipeline(engine->_device);

	vkDestroyShaderModule(engine->device, meshFragShader, nullptr);
	vkDestroyShaderModule(engine->device, meshVertexShader, nullptr);
}

MaterialInstance GLTFMetallic_Roughness::write_material(VulkanTutorialExtension* engine, MaterialPass pass, const MaterialResources& resources)
{
	MaterialInstance matData;
	matData.passType = pass;
	if (pass == MaterialPass::Transparent) {
		matData.pipeline = &transparentPipeline;
	}
	else {
		matData.pipeline = &opaquePipeline;
	}

	int swapChainImageNum = engine->getSwapchainImageNum();

	std::vector<VkDescriptorSetLayout> layouts(swapChainImageNum, materialLayout);
	VkDescriptorSetAllocateInfo allocInfo = vkinit::descriptor_set_allocate_info(engine->descriptorPool, layouts.data(), layouts.size());
	matData.materialSet.resize(swapChainImageNum);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(engine->device, &allocInfo, matData.materialSet.data()));
	//matData.materialSet = descriptorAllocator.allocate(device, materialLayout);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	VkDescriptorBufferInfo bufDescriptor =
		vkinit::descriptor_buffer_info(
			resources.dataBuffer,
			resources.dataBufferOffset,
			sizeof(MaterialConstants));

	VkDescriptorImageInfo texDescriptorColor =
		vkinit::descriptor_image_info(
			resources.colorSampler,
			resources.colorImage,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorMetalRough =
		vkinit::descriptor_image_info(
			resources.metalRoughSampler,
			resources.metalRoughImage,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	for (size_t i = 0; i < swapChainImageNum; i++)
	{
		writeDescriptorSets = {
			// Binding 1 : 
			vkinit::write_descriptor_set(matData.materialSet[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufDescriptor),
			// Binding 2 : 
			vkinit::write_descriptor_set(matData.materialSet[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptorColor),
			// Binding 3 : 
			vkinit::write_descriptor_set(matData.materialSet[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &texDescriptorMetalRough),
		};

		//writer.clear();
		//writer.write_buffer(0, resources.dataBuffer, sizeof(MaterialConstants), resources.dataBufferOffset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		//writer.write_image(1, resources.colorImage, resources.colorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		//writer.write_image(2, resources.metalRoughImage.imageView, resources.metalRoughSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		vkUpdateDescriptorSets(engine->device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		//writer.update_set(device, matData.materialSet);
	}

	return matData;
}
