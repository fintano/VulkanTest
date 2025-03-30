#include "VulkanTutorialExtension.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "VulkanTutorialExtensionImGui.h"
#include "Vk_loader.h"	
#include "vk_descriptor.h"
#include "IrradianceCubeMap.h"
#include "Cube.h"
#include "Sphere.h"
#include "Skybox.h"
#include "SimplePipeline.h"
#include "vk_resource_utils.h"
#include "MaterialTester.h"

static int UniqueBufferIndex = 0;

/**
* GUI에서 사용할 변수들을 모아놓는다. 
*/
int VulkanTutorialExtension::instanceCount = 2;
int VulkanTutorialExtension::maxInstanceCount = instanceCount;
bool VulkanTutorialExtension::useDirectionalLight = false;
int VulkanTutorialExtension::debugDisplayTarget = 0;
float VulkanTutorialExtension::exposure = 1.f;
bool VulkanTutorialExtension::usePointLights = true;
std::array<bool, NR_POINT_LIGHTS> VulkanTutorialExtension::pointLightsSwitch;
float VulkanTutorialExtension::pointLightlinear = 0.09f;
float VulkanTutorialExtension::pointLightQuadratic = 0.032f;
float VulkanTutorialExtension::pointLightIntensity = 1.f;
float VulkanTutorialExtension::directionalLightIntensity = 1.f;

VulkanTutorialExtension::VulkanTutorialExtension()
	: camera({ 5.f, 5.f, 5.f }, { 0.f,1.f,0.f })
{
	pointLightsSwitch[0] = true;
}

void VulkanTutorialExtension::initVulkan()
{
	VulkanTutorial::initVulkan();
	// NOTE : 위치 중요!
	// Descriptor가 초기화 되어야 디폴트 머터리얼이 만들어진다. 그 이후에 이 함수가 불려야한다.
	init_default_data();

	createImGui();
	ImGui_ImplVulkan_CreateFontsTexture();
}

void VulkanTutorialExtension::processInput()
{
	ImGui::stringToDebug.clear();

	VulkanTutorial::processInput();

	double currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);

	ImGui::stringToDebug.append(camera.ToString());
}

void VulkanTutorialExtension::initWindow()
{
	VulkanTutorial::initWindow();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetWindowFocusCallback(window, focusCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	initImGui();
}

void VulkanTutorialExtension::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	bool bRightMouseButtonClicked = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	if (bRightMouseButtonClicked)
	{
		auto app = reinterpret_cast<VulkanTutorialExtension*>(glfwGetWindowUserPointer(window));
		assert(app);

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) {
			// Ignore GLFW input as ImGui is precessing mouse input.
			return;
		}

		app->camera.ProcessMouseMovement(xpos, ypos);
	}
}

void VulkanTutorialExtension::mouseScrollCallback(GLFWwindow* window, double, double yoffset)
{
	bool bRightMouseButtonClicked = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	if (bRightMouseButtonClicked)
	{
		auto app = reinterpret_cast<VulkanTutorialExtension*>(glfwGetWindowUserPointer(window));
		assert(app);

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse) {
			// Ignore GLFW input as ImGui is precessing mouse input.
			return;
		}

		app->camera.ProcessMouseScroll(yoffset);
	}
}

void VulkanTutorialExtension::focusCallback(GLFWwindow* window, int focused)
{
	auto app = reinterpret_cast<VulkanTutorialExtension*>(glfwGetWindowUserPointer(window));
	assert(app);

	app->setWindowFocused(focused);
}

void VulkanTutorialExtension::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) 
	{
		auto app = reinterpret_cast<VulkanTutorialExtension*>(glfwGetWindowUserPointer(window));
		assert(app);

		app->camera.ResetPreoffsets();
	}
}

void VulkanTutorialExtension::createUniformBuffers()
{
	VulkanTutorial::createUniformBuffers();

	{
		globalSceneData.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
		materialConstants.createUniformBuffer(1, device, physicalDevice);
	}

	objectTransformUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	colorUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	materialUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	dirLightUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	pointLightsUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	for (int lightIndex = 0; lightIndex < NR_POINT_LIGHTS; lightIndex++)
	{
		lightTransformUniformBuffer[lightIndex].createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	}
}

void VulkanTutorialExtension::createDescriptorPool()
{
	VulkanTutorial::createDescriptorPool();

	createImGuiDescriptorPool();
}

void VulkanTutorialExtension::createDescriptorSets()
{
	VulkanTutorial::createDescriptorSets();

	{
		GLTFMetallic_Roughness::MaterialResources materialResources {};
		//default the material textures
		materialResources.colorImage = defaultTexture;
		materialResources.colorSampler = textureSampler;
		materialResources.metalRoughImage = defaultTexture;
		materialResources.metalRoughSampler = textureSampler;

		//set the uniform buffer for the material data
		//AllocatedBuffer materialConstants = create_buffer(sizeof(GLTFMetallic_Roughness::MaterialConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		GLTFMetallic_Roughness::MaterialConstants& materialData = materialConstants.clearAndGetFirstInstanceData();
		materialData.colorFactors = glm::vec4{ 1,1,1,1 };
		materialData.metal_rough_factors = glm::vec4{ 1,0.5,0,0 };
		materialConstants.CopyData(UniqueBufferIndex);

		materialResources.dataBuffer = materialConstants.getUniformBuffer(UniqueBufferIndex); //materialConstants.buffer;
		materialResources.dataBufferOffset = 0;

		defaultData.data = metalRoughMaterial.write_material(this, MaterialPass::MainColor, materialResources, descriptorPool);
	}

	{
		createGlobalDescriptorSets();
	}
}

void VulkanTutorialExtension::createGlobalDescriptorSets()
{
	VkDescriptorSetAllocateInfo allocInfo = vkb::initializers::descriptor_set_allocate_info(descriptorPool, &globalDescriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &globalDescriptorSet));

	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	VkDescriptorBufferInfo bufDescriptor =
		vkb::initializers::descriptor_buffer_info(
			globalSceneData.getUniformBuffer(UniqueBufferIndex),
			0,
			sizeof(GPUSceneData));

	writeDescriptorSets = {
		vkb::initializers::write_descriptor_set(globalDescriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufDescriptor),
	};
	
	vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanTutorialExtension::createDescriptorSetsPointLights(UniformBuffer<Transform>& inUniformBuffer, std::vector<VkDescriptorSet>& outDescriptorSets)
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayoutPointLights);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	outDescriptorSets.resize(swapChainImages.size());
	// 각 타입의 여러개 Pool 안에서 레이아웃에 맞춰서 DescriptorSet을 할당한다. 
	if (vkAllocateDescriptorSets(device, &allocInfo, outDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	// 할당된 DescriptorSet에 유니폼 버퍼/샘플러를 쓴다.
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		std::vector<VkWriteDescriptorSet> descriptorWrites;

		inUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanTutorialExtension::createLightingPassDescriptorSets(std::vector<VkDescriptorSet>& outDescriptorSets)
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), lightingPass.descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	outDescriptorSets.resize(swapChainImages.size());
	// 각 타입의 여러개 Pool 안에서 레이아웃에 맞춰서 DescriptorSet을 할당한다. 
	if (vkAllocateDescriptorSets(device, &allocInfo, outDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	// 할당된 DescriptorSet에 유니폼 버퍼/샘플러를 쓴다.
	// binding 순서대로 index가 부여된다. shader와 일치해야한다.
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	VkDescriptorImageInfo pos = vkb::initializers::descriptor_image_info(textureSampler, geometry.position.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo normal = vkb::initializers::descriptor_image_info(textureSampler, geometry.normal.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo albedo = vkb::initializers::descriptor_image_info(textureSampler, geometry.albedo.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo arm = vkb::initializers::descriptor_image_info(textureSampler, geometry.arm.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo diffuseMap = vkb::initializers::descriptor_image_info(textureSampler, irradianceCubeMap->getDiffuseMapImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo specularPrefilterMap = vkb::initializers::descriptor_image_info(textureSampler, irradianceCubeMap->getSpecularMapImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo specularBRDFLUT = vkb::initializers::descriptor_image_info(textureSampler, irradianceCubeMap->getSpecularBRDFLUTImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	auto geometryTextureDescriptor = [&](VkImageView imageView){
		return vkb::initializers::descriptor_image_info(textureSampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	};
	
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		descriptorWrites = {
			vkb::initializers::write_descriptor_set(outDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &pos),
			vkb::initializers::write_descriptor_set(outDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normal),
			vkb::initializers::write_descriptor_set(outDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &albedo),
			vkb::initializers::write_descriptor_set(outDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &arm),
			vkb::initializers::write_descriptor_set(outDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &diffuseMap),
			vkb::initializers::write_descriptor_set(outDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &specularPrefilterMap),
			vkb::initializers::write_descriptor_set(outDescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, &specularBRDFLUT)
		};

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanTutorialExtension::init_default_data()
{
	auto structureFile = loadGltf(this, "models/MetalRoughSpheres.glb");
	assert(structureFile.has_value());

	loadedScenes["structure"] = *structureFile;

	auto gizmoFile = loadGltf(this, "models/gizmo.glb");
	assert(gizmoFile.has_value());

	loadedScenes["gizmo"] = *gizmoFile;
}

void VulkanTutorialExtension::update_scene(uint32_t currentImage)
{
	static glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	// 렌더컨텍스트를 초기화 하고,
	mainDrawContext.OpaqueSurfaces.clear();
	mainDrawContext.TranslucentSurfaces.clear();

	loadedScenes["structure"]->Draw(glm::mat4{ 1.f }, mainDrawContext);
	loadedScenes["gizmo"]->Draw(glm::scale(glm::mat4{ 1.f }, glm::vec3(0.005f)), mainDrawContext);
	//materialTester->draw(mainDrawContext, "gold");

	glm::mat4 viewMat = camera.GetViewMatrix();
	glm::mat4 persMat = glm::perspective(glm::radians(45.f), swapChainExtent.width / (float)(swapChainExtent.height), 0.1f, 100.f);
	persMat[1][1] *= -1;

	GPUSceneData& sceneData = globalSceneData.getFirstInstanceData();
	sceneData = {};
	sceneData.exposureDisplay.x = exposure;
	sceneData.exposureDisplay.y = debugDisplayTarget;
	sceneData.viewPos = camera.Position;
	sceneData.view = viewMat;
	// camera projection
	sceneData.proj = persMat;

	//some default lighting parameters
	sceneData.ambientColor = glm::vec4(.1f);
	sceneData.sunlightColor = glm::vec4(1.f);
	sceneData.sunlightDirection = glm::vec4(0, 1, 0.5, 1.f); 

	// Point Lights
	sceneData.activePointLight.activeLightMask = activePointLightsMask;
	auto& pointLights = sceneData.activePointLight.pointLights;
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		PointLight& pointLight = pointLights[i];
		pointLight.position = pointLightPositions[i];
		pointLight.clq = glm::vec3(1.0f, pointLightlinear, pointLightQuadratic);
		pointLight.colorIntensity = glm::vec4(1.f, 1.f, 1.f, pointLightIntensity);
	}

	// Directional Light
	DirLight dirLight = {};
	if (useDirectionalLight)
	{
		dirLight.colorIntensity = glm::vec4(0.4f, 0.4f, 0.4f, directionalLightIntensity); // darken diffuse light a bit
	}
	dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	sceneData.dirLight = std::move(dirLight);

	globalSceneData.CopyData(currentImage);
}

void VulkanTutorialExtension::updateUniformBuffer(uint32_t currentImage)
{
	VulkanTutorial::updateUniformBuffer(currentImage);

	static glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	glm::mat4 viewMat = camera.GetViewMatrix();
	glm::mat4 persMat = glm::perspective(glm::radians(45.f), swapChainExtent.width / (float)(swapChainExtent.height), 0.1f, 100.f);
	persMat[1][1] *= -1;

	// Object
	Transform& ubo = objectTransformUniformBuffer.clearAndGetFirstInstanceData();
	ubo.model = glm::mat4(1.f);
	ubo.view = viewMat;
	ubo.proj = persMat;

	objectTransformUniformBuffer.CopyData(currentImage);

	ColorUBO& colorUbo = colorUniformBuffer.clearAndGetFirstInstanceData();
	colorUbo.objectColor = glm::vec3(1.0f, 0.0f, 0.0f);
	colorUbo.lightColor = glm::vec3(0.0f, 0.0f, 0.0f);
	colorUbo.viewPos = camera.Position;
	colorUniformBuffer.CopyData(currentImage);

	// Material 
	Material& material = materialUniformBuffer.clearAndGetFirstInstanceData();
	material.ambient = glm::vec3(1.0f, 0.5f, 0.31f);
	material.diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
	material.specular = glm::vec3(1.0f, 0.5f, 0.31f);
	// 유니폼 버퍼 멤버의 타입을 정할 때 float은 잘 안된다. 
	// float을 할거면 vec3/vec4로 해서 컴포넌트로 넘겨주는 것만 작동한다. 
	material.shininess = glm::vec3(32.0f, 0.5f, 0.31f);

	materialUniformBuffer.CopyData(currentImage);

	// Point Lights
	clearPointLightsSwitch();
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		turnPointLightOn(i);
	}
}

void VulkanTutorialExtension::clearUniformBuffer(uint32_t i)
{
	VulkanTutorial::clearUniformBuffer(i);

	objectTransformUniformBuffer.destroy(i);
	colorUniformBuffer.destroy(i);
	materialUniformBuffer.destroy(i);
	dirLightUniformBuffer.destroy(i);
	pointLightsUniformBuffer.destroy(i);
	for (int lightIndex = 0; lightIndex < NR_POINT_LIGHTS; lightIndex++)
	{
		lightTransformUniformBuffer[lightIndex].destroy(i);
	}
	materialConstants.destroy(i);
	globalSceneData.destroy(i);
}

void VulkanTutorialExtension::createDescriptorSetLayouts()
{
	VulkanTutorial::createDescriptorSetLayouts();

	createGlobalDescriptorSetLayout();

	createDescriptorSetLayoutsForPointLights();
	createDescriptorSetLayoutsForObjects();
	createLightingPassDescriptorSetLayout();
}

void VulkanTutorialExtension::createGlobalDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	globalDescriptorSetLayout = vk::desc::createDescriptorSetLayout(device, bindings);
}

void VulkanTutorialExtension::createDescriptorSetLayoutsForPointLights()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	//	Transform
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	descriptorSetLayoutPointLights = vk::desc::createDescriptorSetLayout(device, bindings);
}

void VulkanTutorialExtension::createDescriptorSetLayoutsForObjects()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	//	Transform
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, bindings);
	//	TexSampler
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	colorUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	materialUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	dirLightUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	pointLightsUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);

	descriptorSetLayout = vk::desc::createDescriptorSetLayout(device, bindings);
}

void VulkanTutorialExtension::createLightingPassDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	//	position
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	normal
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	color 
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	arm
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	// diffuse map
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	// specular prefilter map
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	// brdf lut 2d
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);

	lightingPass.descriptorSetLayout = vk::desc::createDescriptorSetLayout(device, bindings);
}

void VulkanTutorialExtension::createGraphicsPipelines()
{
	VulkanTutorial::createGraphicsPipelines();

	{
		metalRoughMaterial.build_pipelines(this);
	}

	createPipelineLayout(descriptorSetLayoutPointLights, pipelineLayoutPointLights);
	createPipelineLayout(descriptorSetLayout, pipelineLayoutObject);
	//createPipelineLayout(lightingPass.descriptorSetLayout, lightingPass.pipelineLayout);
	
	/*
	* Default 
	*/

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = msaaSamples;

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // lower depth == closer
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	// shader modules
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencilInfo;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.subpass = 0; // index of the subpass where this graphics pipeline will be used.
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	// point light objects
	auto vertShaderModule = Utils::loadShader("shaders/shadervert.spv", device);
	auto fragShaderModule = Utils::loadShader("shaders/ForwardPassfrag.spv", device);

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	Vertex::getBindingDescriptions(bindingDescriptions);

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	Vertex::getAttributeDescriptions(attributeDescriptions);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	// the number of vertex buffer.
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	auto DefaultPipelineColorBlendAttachmentState = [](){
		VkPipelineColorBlendAttachmentState State{};
		State.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		State.blendEnable = VK_FALSE;
		return State;
	};

	std::array<VkPipelineColorBlendAttachmentState, 1> colorBlendAttachments = {
		DefaultPipelineColorBlendAttachmentState()
	};
	auto& colorBlendAttachment = colorBlendAttachments[0];
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;  // 소스 색상 블렌딩 팩터
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // 대상 색상 블렌딩 팩터
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;  // 색상 블렌딩 연산
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // 소스 알파 블렌딩 팩터
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // 대상 알파 블렌딩 팩터
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;  // 알파 블렌딩 연산

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = colorBlendAttachments.size();
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.layout = pipelineLayoutPointLights;
	pipelineInfo.renderPass = forward.renderPass;
	pipelineInfo.pColorBlendState = &colorBlending;

	// vk_engine의 코드로 일단은 갈음한다.
	//if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &forward.pipeline) != VK_SUCCESS)
	//{
	//	throw std::runtime_error("failed to create graphics pipeline");
	//}

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);

	/**
	* For objects
	*/ 

	Instance::getBindingDescriptions(bindingDescriptions);
	Instance::getAttributeDescriptions(attributeDescriptions);

	vertShaderModule = Utils::loadShader("shaders/ObjectShadervert.spv", device);
	fragShaderModule = Utils::loadShader("shaders/ObjectShaderfrag.spv", device);

	vertShaderStageInfo.module = vertShaderModule;
	fragShaderStageInfo.module = fragShaderModule;

	shaderStages[0] = vertShaderStageInfo;
	shaderStages[1] = fragShaderStageInfo;

	vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	std::array<VkPipelineColorBlendAttachmentState, 3> deferredColorBlendAttachments =
	{
		DefaultPipelineColorBlendAttachmentState(),
		DefaultPipelineColorBlendAttachmentState(),
		DefaultPipelineColorBlendAttachmentState()
	};

	colorBlending.attachmentCount = deferredColorBlendAttachments.size();
	colorBlending.pAttachments = deferredColorBlendAttachments.data();

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.layout = pipelineLayoutObject;
	pipelineInfo.renderPass = geometry.renderPass;
	pipelineInfo.pColorBlendState = &colorBlending;

	//if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineObject) != VK_SUCCESS)
	//{
	//	throw std::runtime_error("failed to create graphics pipeline");
	//}

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);

	/**
	* For lightpass
	*/

	std::array<VkDescriptorSetLayout, 2> layouts = { globalDescriptorSetLayout, lightingPass.descriptorSetLayout };
	VkPipelineLayoutCreateInfo mesh_layout_info = vkb::initializers::pipeline_layout_create_info(layouts.size());
	mesh_layout_info.pSetLayouts = layouts.data();
	
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &mesh_layout_info, nullptr, &lightingPass.pipelineLayout));
	 
	vertShaderModule = Utils::loadShader("shaders/LightingPassvert.spv", device);
	fragShaderModule = Utils::loadShader("shaders/Pbrfrag.spv", device);
	
	vertShaderStageInfo.module = vertShaderModule;
	fragShaderStageInfo.module = fragShaderModule;

	shaderStages[0] = vertShaderStageInfo;
	shaderStages[1] = fragShaderStageInfo;

	// empty input state
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineColorBlendAttachmentState lightingColorBlendAttachment{};
	lightingColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	lightingColorBlendAttachment.blendEnable = VK_FALSE;

	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &lightingColorBlendAttachment;

	// NOTE : 이게 핵심이다. BACK_FACE_CULLING으로 하니까 안나오네. 
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.layout = lightingPass.pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.pDepthStencilState = nullptr;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &lightingPass.pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline");
	}

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void VulkanTutorialExtension::recordCommandBuffer(VkCommandBuffer commandBuffer, size_t index)
{	
	// PBR 디버그 용
	//irradianceCubeMap->draw(commandBuffer, this);

	VulkanTutorial::recordCommandBuffer(commandBuffer, index);
}

void VulkanTutorialExtension::recordRenderPassCommands(VkCommandBuffer commandBuffer, size_t i)
{
	VulkanTutorial::recordRenderPassCommands(commandBuffer, i);

	for (const RenderObject& r : mainDrawContext.OpaqueSurfaces)
	{
		drawRenderObject(commandBuffer, i, r);
	}
}

void VulkanTutorialExtension::recordLightingRenderPassCommands(VkCommandBuffer commandBuffer, size_t i)
{
	// Lighting Pass
	vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPass.pipeline);
	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPass.pipelineLayout, 0, 1, &globalDescriptorSet, 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPass.pipelineLayout, 1, 1, &lightingPass.descriptorSets[i], 0, nullptr);
	// Final composition
	// This is done by simply drawing a full screen quad
	// The fragment shader then combines the geometry attachments into the final image
	// Note: Also used for debug display if debugDisplayTarget > 0
	vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
}

void VulkanTutorialExtension::recordForwardPassCommands(VkCommandBuffer commandBuffer, size_t i)
{
	VulkanTutorial::recordForwardPassCommands(commandBuffer, i);

	VkViewport viewport = vkb::initializers::viewport((float)swapChainExtent.width, (float)swapChainExtent.height, 0.0f, 1.0f);
	VkRect2D scissor = vkb::initializers::rect2D(swapChainExtent.width, swapChainExtent.height, 0, 0);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	drawRenderObject(commandBuffer, i, skybox->getRenderObject());

	for (const RenderObject& r : mainDrawContext.TranslucentSurfaces)
	{
		drawRenderObject(commandBuffer, i, r);
	}
}

void VulkanTutorialExtension::drawRenderObject(VkCommandBuffer commandBuffer, size_t i, const RenderObject& draw)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->layout, 0, 1, &globalDescriptorSet, 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->layout, 1, 1, &draw.material->materialSet[i], 0, nullptr);

	VkBuffer vertexBuffers[]{ draw.vertexBuffer };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, draw.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	GPUDrawPushConstants pushConstants;
	pushConstants.model = draw.transform;
	vkCmdPushConstants(commandBuffer, draw.material->pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

	vkCmdDrawIndexed(commandBuffer, draw.indexCount, 1, draw.firstIndex, 0, 0);
}

void VulkanTutorialExtension::createInstanceBuffer(uint32_t imageIndex)
{
	VkDeviceSize bufferSize = sizeof(instances[0]) * instanceCount;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, instances.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instanceBuffers[imageIndex], instanceBufferMemories[imageIndex]);
	copyBuffer(stagingBuffer, instanceBuffers[imageIndex], bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanTutorialExtension::setWindowFocused(int inFocused)
{
	focused = inFocused;
}

void VulkanTutorialExtension::loadModels()
{
	VulkanTutorial::loadModels();

	//loadModel(cube.objPath, cube.vertices, cube.indices);

	static glm::vec3 cubePositions[] = {
			glm::vec3(0.0f,  0.0f,  0.0f),
			glm::vec3(0.0f,  2.0f,  0.0f),
			glm::vec3(2.0f,  5.0f, -15.0f),
			glm::vec3(-1.5f, -2.2f, -2.5f),
			glm::vec3(-3.8f, -2.0f, -12.3f),
			glm::vec3(2.4f, -0.4f, -3.5f),
			glm::vec3(-1.7f,  3.0f, -7.5f),
			glm::vec3(1.3f, -2.0f, -2.5f),
			glm::vec3(1.5f,  2.0f, -2.5f),
			glm::vec3(1.5f,  0.2f, -1.5f),
			glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	static int instanceCount = sizeof(cubePositions) / sizeof(cubePositions[0]);
	maxInstanceCount = instanceCount;

	for (unsigned int i = 0; i < instanceCount; i++)
	{
		Instance instance;

		glm::mat4 identity = glm::mat4(1.0f);
		// the obj model file is rotated incorrectly, so it needs to be fixed.
		glm::mat4 xRotation = glm::rotate(identity, glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 yRotation = glm::rotate(identity, glm::radians(270.f), glm::vec3(0.f, 1.f, 0.f));
		instance.model = yRotation * xRotation;

		instance.model = glm::translate(instance.model, cubePositions[i]);
		float angle = 20.0f * i;
		instance.model = glm::rotate(instance.model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

		instances.push_back(instance);
	}
}

void VulkanTutorialExtension::createBuffers()
{
	VulkanTutorial::createBuffers();

	for (int i = 0; i < INSTANCE_BUFFER_COUNT; i++)
	{
		createInstanceBuffer(i);
	}
}

bool VulkanTutorialExtension::instanceCountChanged()
{
	return previousInstanceCount != instanceCount;
}

void VulkanTutorialExtension::recreateSwapChain()
{
	VulkanTutorial::recreateSwapChain();

	ImGui_ImplVulkan_SetMinImageCount(swapChainImages.size());
}

void VulkanTutorialExtension::preDrawFrame(uint32_t imageIndex)
{
	VulkanTutorial::preDrawFrame(imageIndex);

	update_scene(imageIndex);

	if (pointLightSwitchChanged(imageIndex))
	{
		//vkFreeCommandBuffers(device, commandPool, 1, &commandBuffers[imageIndex]);
		createCommandBuffer(imageIndex);
		previousActivePointLightsMask[imageIndex] = activePointLightsMask;
	}

	drawImGui(imageIndex);
}

void VulkanTutorialExtension::recreateInstanceBuffer(uint32_t imageIndex)
{
	vkDestroyBuffer(device, instanceBuffers[imageIndex], nullptr);
	vkFreeMemory(device, instanceBufferMemories[imageIndex], nullptr);

	createInstanceBuffer(imageIndex);
}

void VulkanTutorialExtension::drawFrame()
{
	VulkanTutorial::drawFrame();
}

void VulkanTutorialExtension::postDrawFrame(uint32_t imageIndex)
{
	VulkanTutorial::postDrawFrame(imageIndex);

	if (instanceCountChanged())
	{
		previousInstanceCount = instanceCount;
		usingInstanceBufferIndex = (usingInstanceBufferIndex + 1 ) % INSTANCE_BUFFER_COUNT;
		recreateInstanceBuffer(usingInstanceBufferIndex);
		recreateSwapChain();
	}
}

void VulkanTutorialExtension::createCommandPool()
{
	VulkanTutorial::createCommandPool();

	createImGuiCommandPool();
}

void VulkanTutorialExtension::onPostInitVulkan()
{
	irradianceCubeMap = std::make_shared<IrradianceCubeMap>(device, descriptorPool);
	irradianceCubeMap->initialize(this);

	createLightingPassDescriptorSets(lightingPass.descriptorSets);

	skybox = std::make_shared<Skybox>();
	skybox->initialize(this);

	materialTester = std::make_shared<MaterialTester>();
	materialTester->init(this);
	materialTester->createMaterial(this, 
		"gold",
		"textures/pbr/gold/albedo.png",
		"textures/pbr/gold/normal.png",
		"textures/pbr/gold/metallic.png",
		"textures/pbr/gold/roughness.png",
		"textures/pbr/gold/ao.png"
	);

	VulkanTutorial::onPostInitVulkan();
}

void VulkanTutorialExtension::createCommandBuffers()
{
	VulkanTutorial::createCommandBuffers();

	createImGuiCommandBuffers();
	previousActivePointLightsMask.resize(swapChainImages.size());
}

void VulkanTutorialExtension::createFrameBuffers()
{
	VulkanTutorial::createFrameBuffers();

	createImGuiFrameBuffers();
}

void VulkanTutorialExtension::createRenderPass()
{
	VulkanTutorial::createRenderPass();

	createImGuiRenderPass();
}
void VulkanTutorialExtension::cleanUpSwapchain()
{
	vkDestroyPipelineLayout(device, pipelineLayoutObject, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayoutPointLights, nullptr);
	vkDestroyPipeline(device, graphicsPipelineObject, nullptr);
	vkDestroyPipeline(device, graphicsPipelinePointLights, nullptr);

	cleanUpImGuiSwapchain();

	VulkanTutorial::cleanUpSwapchain();
}

void VulkanTutorialExtension::cleanUp()
{
	glfwSetWindowUserPointer(window, nullptr);

	cleanUpImGui();

	for (auto instanceBuffer : instanceBuffers)
	{
		vkDestroyBuffer(device, instanceBuffer, nullptr);
	}

	for (auto instanceBufferMemory : instanceBufferMemories)
	{
		vkFreeMemory(device, instanceBufferMemory, nullptr);
	}
	vkDestroyDescriptorSetLayout(device, descriptorSetLayoutPointLights, nullptr);
	vkDestroyDescriptorSetLayout(device, globalDescriptorSetLayout, nullptr);

	//vkDestroyPipeline(device, defaultData.data.pipeline->pipeline, nullptr);
	//vkDestroyPipelineLayout(device, defaultData.data.pipeline->layout, nullptr);

	metalRoughMaterial.clear_resources(device);

	// make sure the gpu has stopped doing its things
	vkDeviceWaitIdle(device);

	loadedScenes.clear();
	irradianceCubeMap.reset();
	skybox->cleanup(device);
	materialTester->cleanUp(device);
	VulkanTutorial::cleanUp();
}

bool VulkanTutorialExtension::pointLightSwitchChanged(uint32_t index)
{
	return previousActivePointLightsMask[index] != activePointLightsMask;
}

void VulkanTutorialExtension::clearPointLightsSwitch()
{
	activePointLightsMask = 0;
}

void VulkanTutorialExtension::turnPointLightOn(int index)
{
	if (index >= 0 && index < NR_POINT_LIGHTS)
	{
		activePointLightsMask |= (pointLightsSwitch[index] << index);
	}
}

bool VulkanTutorialExtension::isLightOn(int index)
{
	return activePointLightsMask & (1 << index);
}