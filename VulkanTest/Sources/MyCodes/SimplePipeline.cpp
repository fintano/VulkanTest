#include "SimplePipeline.h"
#include "VulkanTutorialExtension.h" 
#include "VulkanTools.h"       
#include "vk_initializers.h"
#include "vk_resource_utils.h"

// 정적 변수 초기화
std::unordered_map<uint32_t, size_t> SimplePipeline::TypeSizeTracker::typeSizes;

// TypeSizeTracker 메소드 구현
template<typename T>
void SimplePipeline::TypeSizeTracker::registerType(uint32_t binding) {
    typeSizes[binding] = sizeof(T);
}

size_t SimplePipeline::TypeSizeTracker::getRegisteredSize(uint32_t binding) {
    auto it = typeSizes.find(binding);
    if (it != typeSizes.end()) {
        return it->second;
    }
    return 0; // 기본값
}

// DescriptorBuilder 메소드 구현
SimplePipeline::DescriptorBuilder& SimplePipeline::DescriptorBuilder::addTexture(VkShaderStageFlags stageFlags) {
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = currentBinding;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = stageFlags;
    bindings.push_back(binding);
    descriptorTypes.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    currentBinding++;
    return *this;
}

VkDescriptorSetLayout SimplePipeline::DescriptorBuilder::buildLayout(VkDevice device) const {
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VkDescriptorSetLayout layout;
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout");
    }

    return layout;
}

VkDescriptorType SimplePipeline::DescriptorBuilder::getDescriptorType(uint32_t binding) const {
    if (binding >= descriptorTypes.size()) {
        throw std::runtime_error("Binding index out of range");
    }
    return descriptorTypes[binding];
}

size_t SimplePipeline::DescriptorBuilder::getBindingCount() const {
    return bindings.size();
}

const std::vector<VkDescriptorSetLayoutBinding>& SimplePipeline::DescriptorBuilder::getBindings() const {
    return bindings;
}

// SimplePipeline 메소드 구현
SimplePipeline::SimplePipeline(VkExtent2D viewportExtent, std::string vertShaderPath, std::string fragShaderPath,
    RenderType renderType, VkDescriptorPool pool, int swapchainImageNum)
    : viewportExtent(viewportExtent)
    , vertShaderPath(vertShaderPath)
    , fragShaderPath(fragShaderPath)
    , renderType(renderType)
    , descriptorPool(pool)
    , swapchainImageNum(swapchainImageNum)
{
}

SimplePipeline::DescriptorBuilder& SimplePipeline::getDescriptorBuilder() {
    return descriptorBuilder;
}

// 추가 디스크립터 셋 레이아웃을 파이프라인에 포함
void SimplePipeline::addExternalDescriptorSetLayout(VkDescriptorSetLayout externalLayout) {
    externalLayouts.push_back(externalLayout);
}

void SimplePipeline::addExternalDescriptorSet(VkDescriptorSet externalDescriptorSet) {
    externalDescriptorSets.push_back(externalDescriptorSet);
}

void SimplePipeline::addExternalDescriptorSet(const std::vector<VkDescriptorSet>& externalDescriptorSet) {
    externalDescriptorSets.insert(externalDescriptorSets.end(),externalDescriptorSet.begin(), externalDescriptorSet.end());
}

void SimplePipeline::createDescriptorSetLayout(VkDevice device) {
    // 디스크립터 빌더로 레이아웃 생성
    layout = descriptorBuilder.buildLayout(device);
}

void SimplePipeline::createVertexInput() {
    Vertex::getBindingDescriptions(bindingDescriptions);
    Vertex::getAttributeDescriptions(attributeDescriptions);
}

void SimplePipeline::allocateDescriptorSet(VkDevice device) {
    if (layout == VK_NULL_HANDLE) {
        throw std::runtime_error("Descriptor layout not created");
    }

    std::vector<VkDescriptorSetLayout> layouts(swapchainImageNum, layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;  // 내부에 저장된 풀 사용
    allocInfo.descriptorSetCount = layouts.size();
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapchainImageNum);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }
}

void SimplePipeline::buildPipeline(VulkanTutorialExtension* engine, std::function<void(VkGraphicsPipelineCreateInfo&)> modifyFunc) {
    VkDevice device = engine->getDevice();

    createDescriptorSetLayout(device);

    std::vector<VkDescriptorSetLayout> layouts = externalLayouts; // 외부 레이아웃을 먼저 추가
    layouts.push_back(layout); // 자신의 레이아웃을 마지막에 추가

    VkPipelineLayoutCreateInfo mesh_layout_info = vkb::initializers::pipeline_layout_create_info(layouts.data(), layouts.size());
    mesh_layout_info.pPushConstantRanges = matrixRanges.data();
    mesh_layout_info.pushConstantRangeCount = matrixRanges.size();

    VkPipelineLayout newLayout;
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &mesh_layout_info, nullptr, &newLayout));
    pipeline.layout = newLayout;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    // create pipeline
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vkb::initializers::pipeline_input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    VkPipelineViewportStateCreateInfo viewportState = vkb::initializers::pipeline_viewport_state_create_info(1, 1, 0);
    VkPipelineRasterizationStateCreateInfo rasterizationState = vkb::initializers::pipeline_rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
    VkPipelineMultisampleStateCreateInfo multisampleState = vkb::initializers::pipeline_multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, 0);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = vkb::initializers::pipeline_depth_stencil_state_create_info(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
    VkPipelineDynamicStateCreateInfo dynamicState = vkb::initializers::pipeline_dynamic_state_create_info(dynamicStates);

    VkGraphicsPipelineCreateInfo pipelineCI = vkb::initializers::pipeline_create_info(newLayout, renderType == RenderType::forward ? engine->forward.renderPass : engine->geometry.renderPass);
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamicState;
    
    // Viewport
    VkViewport viewport = vkb::initializers::viewport((float)viewportExtent.width, (float)viewportExtent.height, 0.f, 1.f);
    VkRect2D scissor = vkb::initializers::rect2D(viewportExtent.width, viewportExtent.height, 0, 0);
    viewportState.pViewports = &viewport;
    viewportState.pScissors = &scissor;

    std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
    VkPipelineColorBlendStateCreateInfo colorBlendState;

    if (renderType == RenderType::deferred) {
        blendAttachmentStates = {
            vkb::initializers::pipeline_color_blend_attachment_state(0xf, VK_FALSE),
            vkb::initializers::pipeline_color_blend_attachment_state(0xf, VK_FALSE),
            vkb::initializers::pipeline_color_blend_attachment_state(0xf, VK_FALSE),
            vkb::initializers::pipeline_color_blend_attachment_state(0xf, VK_FALSE)
        };
    }
    else {
        blendAttachmentStates = { vkb::initializers::pipeline_color_blend_attachment_state(0xf, VK_TRUE) };
        blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
        blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
    }

    colorBlendState = vkb::initializers::pipeline_color_blend_state_create_info(blendAttachmentStates.size(), blendAttachmentStates.data());
    pipelineCI.pColorBlendState = &colorBlendState;

    // Shaders
    VkShaderModule meshVertexShader = Utils::loadShader(vertShaderPath.c_str(), device);
    VkShaderModule meshFragShader = Utils::loadShader(fragShaderPath.c_str(), device);

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
    createVertexInput();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = vkb::initializers::pipeline_vertex_input_state_create_info();
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    pipelineCI.pVertexInputState = &vertexInputInfo;

    // 사용자 콜백 함수로 수정 가능하게
    if (modifyFunc) {
        modifyFunc(pipelineCI);
    }

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline.pipeline));

    vkDestroyShaderModule(device, meshVertexShader, nullptr);
    vkDestroyShaderModule(device, meshFragShader, nullptr);

    // 디스크립터 셋 할당
    allocateDescriptorSet(device);
}

void SimplePipeline::updateTextureDescriptor(VkDevice device, uint32_t index, uint32_t binding,
    VkImageView imageView, VkSampler sampler,
    VkImageLayout imageLayout) {
    if (descriptorBuilder.getDescriptorType(binding) != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        throw std::runtime_error("Binding is not a texture descriptor");
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = imageLayout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSets[index];  // 내부 디스크립터 셋 사용
    write.dstBinding = binding;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void SimplePipeline::updateBufferDescriptor(VkDevice device, uint32_t index, uint32_t binding,
    VkBuffer buffer, VkDeviceSize offset) {
    VkDescriptorType type = descriptorBuilder.getDescriptorType(binding);
    if (type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
        type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        throw std::runtime_error("Binding is not a buffer descriptor");
    }

    // 등록된 타입 크기 사용
    VkDeviceSize range = TypeSizeTracker::getRegisteredSize(binding);
    if (range == 0) {
        throw std::runtime_error("Buffer size not registered for binding");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSets[index];  // 내부 디스크립터 셋 사용
    write.dstBinding = binding;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void SimplePipeline::updateBufferDescriptor(VkDevice device, uint32_t index, uint32_t binding,
    VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {

    VkDescriptorType type = descriptorBuilder.getDescriptorType(binding);
    if (type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
        type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        throw std::runtime_error("Binding is not a buffer descriptor");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSets[index];  // 내부 디스크립터 셋 사용
    write.dstBinding = binding;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

std::shared_ptr<MaterialInstance> SimplePipeline::makeMaterial() {
	
    auto material = std::make_shared<MaterialInstance>();
	material->pipeline = &pipeline;  // 자신의 MaterialPipeline을 가리킴
	material->materialSet = descriptorSets;  // 자신의 셋 추가
	material->passType = renderType == RenderType::forward ? MaterialPass::Transparent : MaterialPass::MainColor;

	return material;
}

// 푸시 상수 범위 추가
void SimplePipeline::addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset) {
    VkPushConstantRange range{};
    range.stageFlags = stageFlags;
    range.offset = offset;
    range.size = size;
    matrixRanges.push_back(range);
}

void SimplePipeline::cleanup(VkDevice device) {
    // 파이프라인 객체 정리
    if (pipeline.pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, pipeline.pipeline, nullptr);
        pipeline.pipeline = VK_NULL_HANDLE;
    }

    // 파이프라인 레이아웃 정리
    if (pipeline.layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
        pipeline.layout = VK_NULL_HANDLE;
    }

    // 디스크립터 셋 레이아웃 정리
    if (layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
        layout = VK_NULL_HANDLE;
    }
}

void SimplePipelineEmptyInput::createVertexInput()
{
}

void SimplePipelinePosOnly::createVertexInput()
{
    VertexOnlyPos::getBindingDescriptions(bindingDescriptions);
    VertexOnlyPos::getAttributeDescriptions(attributeDescriptions);
}

void SimplePipelinePosTex::createVertexInput()
{
    VertexOnlyTex::getBindingDescriptions(bindingDescriptions);
    VertexOnlyTex::getAttributeDescriptions(attributeDescriptions);
}