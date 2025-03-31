#include "Skybox.h"
#include "vk_engine.h"
#include "Vk_loader.h"
#include "vk_initializers.h"
#include "vk_descriptor.h"
#include "VulkanTutorialExtension.h"
#include "Cube.h"
#include "Vertex.h"
#include "SimplePipeline.h"
#include "IrradianceCubeMap.h"

void Skybox::initialize(VulkanTutorialExtension* engine)
{
	assert(engine->irradianceCubeMap && engine->irradianceCubeMap->getCubeImageView());

    pipeline = std::make_shared<SimplePipelinePosOnly>(engine->getSwapchainExtent(),
        "shaders/skyboxvert.spv",
        "shaders/skyboxfrag.spv",
        SimplePipeline::RenderType::forward,
        engine->descriptorPool, engine->getSwapchainImageNum());

	pipeline->addExternalDescriptorSetLayout(engine->getGlobalDescriptorSetLayout());
	pipeline->getDescriptorBuilder().addTexture();
	pipeline->addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GPUDrawPushConstants), 0);

	pipeline->buildPipeline(engine, [](VkGraphicsPipelineCreateInfo& pipelineCI) {
		VkPipelineRasterizationStateCreateInfo* rasterizationState = 
		 const_cast<VkPipelineRasterizationStateCreateInfo*>(pipelineCI.pRasterizationState);
		rasterizationState->cullMode = VK_CULL_MODE_FRONT_BIT;

		VkPipelineDepthStencilStateCreateInfo* depthState =
			const_cast<VkPipelineDepthStencilStateCreateInfo*>(pipelineCI.pDepthStencilState);
		depthState->depthTestEnable = VK_TRUE;
		depthState->depthWriteEnable = VK_FALSE;
		depthState->depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	});
    
	irradianceCubeMap = engine->irradianceCubeMap->getCubeImageView();

	for (int i = 0; i < engine->getSwapchainImageNum(); i++)
	{
		pipeline->updateTextureDescriptor(engine->getDevice(), i, 0, irradianceCubeMap->imageView, engine->getDefaultTextureSampler());
	}

	cube = std::make_shared<Cube<VertexOnlyPos>>();
	cube->createMesh(engine);

	mesh = std::make_shared<MeshAsset<VertexOnlyPos>>();
	mesh->meshBuffers = cube->mesh;

	renderObject.indexCount = mesh->meshBuffers.indexBuffer.indices.size();
	renderObject.firstIndex = 0;
	renderObject.vertexBuffer = mesh->meshBuffers.vertexBuffer.Buffer;
	renderObject.indexBuffer = mesh->meshBuffers.indexBuffer.Buffer;
	renderObject.material = pipeline->makeMaterial();
}

void Skybox::update(uint32_t currentImage)
{
	// vkUpdateDescriptorSets를 매 프레임 부르는게 아니다. 
	// 이건 한번만 부르고 여기에 바인딩된 데이터를 업데이트하는 것이다. 
	// 여기서 이렇게 update를 매 프레임할 필요가 없다. 
}

void Skybox::cleanup(VkDevice device)
{
	cube->cleanUp(device);
	pipeline->cleanup(device);
}

bool Skybox::isValid()
{
	return !!irradianceCubeMap;
}
