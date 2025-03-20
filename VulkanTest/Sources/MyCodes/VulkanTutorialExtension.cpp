#include "VulkanTutorialExtension.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "VulkanTutorialExtensionImGui.h"
#include "Vk_loader.h"	
#include "vk_descriptor.h"

static int UniqueBufferIndex = 0;

/**
* GUI에서 사용할 변수들을 모아놓는다. 
*/
int VulkanTutorialExtension::instanceCount = 2;
int VulkanTutorialExtension::maxInstanceCount = instanceCount;
bool VulkanTutorialExtension::useDirectionalLight = false;
bool VulkanTutorialExtension::debugGBuffers = false;
bool VulkanTutorialExtension::usePointLights = true;
std::array<bool, NR_POINT_LIGHTS> VulkanTutorialExtension::pointLightsSwitch;
float VulkanTutorialExtension::pointLightlinear = 0.09f;
float VulkanTutorialExtension::pointLightQuadratic = 0.032f;

VulkanTutorialExtension::VulkanTutorialExtension()
	: camera({ 5.f, 5.f, 5.f }, { 0.f,1.f,0.f })
{
	pointLightsSwitch[0] = true;

	//cube.objPath = "models/cube.obj";
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
		GLTFMetallic_Roughness::MaterialResources materialResources;
		//default the material textures
		materialResources.colorImage = textureImageView;
		materialResources.colorSampler = textureSampler;
		materialResources.metalRoughImage = textureImageView;
		materialResources.metalRoughSampler = textureSampler;

		//set the uniform buffer for the material data
		//AllocatedBuffer materialConstants = create_buffer(sizeof(GLTFMetallic_Roughness::MaterialConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		GLTFMetallic_Roughness::MaterialConstants& materialData = materialConstants.clearAndGetFirstInstanceData();
		materialData.colorFactors = glm::vec4{ 1,1,1,1 };
		materialData.metal_rough_factors = glm::vec4{ 1,0.5,0,0 };
		materialConstants.CopyData(UniqueBufferIndex);

		//write the buffer
		//GLTFMetallic_Roughness::MaterialConstants* sceneUniformData = (GLTFMetallic_Roughness::MaterialConstants*)materialConstants.allocation->GetMappedData();
		//MaterialConstant.colorFactors = glm::vec4{ 1,1,1,1 };
		//MaterialConstant.metal_rough_factors = glm::vec4{ 1,0.5,0,0 };

		//_mainDeletionQueue.push_function([=, this]() {
		//	destroy_buffer(materialConstants);
		//	});


		materialResources.dataBuffer = materialConstants.getUniformBuffer(UniqueBufferIndex); //materialConstants.buffer;
		materialResources.dataBufferOffset = 0;

		defaultData.data = metalRoughMaterial.write_material(this, MaterialPass::MainColor, materialResources, descriptorPool);
	}

	{
		createGlobalDescriptorSets();
	}

	/*for (int lightIndex = 0; lightIndex < NR_POINT_LIGHTS; lightIndex++)
	{
		createDescriptorSetsPointLights(lightTransformUniformBuffer[lightIndex], descriptorSetsPointLights[lightIndex]);
	}*/
	createDescriptorSetsObject(descriptorSetsObject);
	createLightingPassDescriptorSets(lightingPass.descriptorSets);
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

void VulkanTutorialExtension::createDescriptorSetsObject(std::vector<VkDescriptorSet>& outDescriptorSets)
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);

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
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		std::vector<VkWriteDescriptorSet> descriptorWrites;

		objectTransformUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);

		VkDescriptorImageInfo ImageInfo = CreateDescriptorImageInfo(textureImageView, textureSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		CreateWriteDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, outDescriptorSets[i], &ImageInfo, nullptr, descriptorWrites);

		colorUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);
		materialUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);
		dirLightUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);
		pointLightsUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);

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
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		std::vector<VkWriteDescriptorSet> descriptorWrites;
		
		// colorUBO
		colorUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);
		
		// dirLightUniform
		dirLightUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);
		
		// pointLightsUniform
		pointLightsUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);

		// position
		VkDescriptorImageInfo positionImageInfo = CreateDescriptorImageInfo(geometry.position.imageView, textureSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		CreateWriteDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, outDescriptorSets[i], &positionImageInfo, nullptr, descriptorWrites);

		// normal 
		VkDescriptorImageInfo normalImageInfo = CreateDescriptorImageInfo(geometry.normal.imageView, textureSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		CreateWriteDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, outDescriptorSets[i], &normalImageInfo, nullptr, descriptorWrites);

		// color + specular
		VkDescriptorImageInfo colorSpecularImageInfo = CreateDescriptorImageInfo(geometry.albedo.imageView, textureSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		CreateWriteDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, outDescriptorSets[i], &colorSpecularImageInfo, nullptr, descriptorWrites);

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void VulkanTutorialExtension::init_default_data()
{
	auto structureFile = loadGltf(this, "models/CompareMetallic.glb");
	assert(structureFile.has_value());

	loadedScenes["structure"] = *structureFile;
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

	// 어떤 매쉬를 드로우 할지 결정하는 곳.
	for (int lightIndex = 0; lightIndex < NR_POINT_LIGHTS; lightIndex++)
	{
		if (isLightOn(lightIndex))
		{
			glm::mat4 lightTransform = glm::translate(glm::mat4(1.f), pointLightPositions[lightIndex]);
			//lightTransform = glm::scale(lightTransform, glm::vec3(0.005f));

			//for (int x = -3; x < 3; x++)
			//{
			//	glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3{ 0.2 });
			//	glm::mat4 translation = glm::translate(glm::mat4(1.f), glm::vec3{ x, 1, 0 });

			//	loadedNodes["Mesh"]->Draw(translation * scale * lightTransform, mainDrawContext);
			//}
		}
	}

	loadedScenes["structure"]->Draw(glm::mat4{ 1.f }, mainDrawContext);

	glm::mat4 viewMat = camera.GetViewMatrix();
	glm::mat4 persMat = glm::perspective(glm::radians(45.f), swapChainExtent.width / (float)(swapChainExtent.height), 0.1f, 100.f);
	persMat[1][1] *= -1;

	GPUSceneData& sceneData = globalSceneData.getFirstInstanceData();

	sceneData.viewPos = camera.Position;
	sceneData.view = viewMat;//glm::translate(glm::vec3{ 0,0,-5 });
	// camera projection
	sceneData.proj = persMat;//glm::perspective(glm::radians(70.f), (float)_windowExtent.width / (float)_windowExtent.height, 10000.f, 0.1f);

	//some default lighting parameters
	sceneData.ambientColor = glm::vec4(.1f);
	sceneData.sunlightColor = glm::vec4(1.f);
	sceneData.sunlightDirection = glm::vec4(0, 1, 0.5, 1.f);

	//pointLights;
	sceneData.activePointLight.activeLightMask = activePointLightsMask;
	auto& pointLights = sceneData.activePointLight.pointLights;
	for (int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		PointLight& pointLight = pointLights[i];
		pointLight.position = pointLightPositions[i];
		pointLight.clq = glm::vec3(1.0f, pointLightlinear, pointLightQuadratic);
		pointLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
		pointLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
		pointLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	}

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
	colorUbo.Debug.x = debugGBuffers;
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

	// Directional Light;
	DirLight& dirLight = dirLightUniformBuffer.clearAndGetFirstInstanceData();
	if (useDirectionalLight)
	{
		dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
		dirLight.diffuse = glm::vec3(0.4f, 0.4f, 0.4f); // darken diffuse light a bit
		dirLight.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	}
	dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);

	// Point Lights
	std::array<PointLight, NR_POINT_LIGHTS > pointLights;
	clearPointLightsSwitch();

	for (int i = 0; i < NR_POINT_LIGHTS; i++)
	{
		PointLight pointLight;
		pointLight.position = pointLightPositions[i];
		if (usePointLights)
		{
			pointLight.clq = glm::vec3(1.0f, pointLightlinear, pointLightQuadratic);
			pointLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
			pointLight.diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
			pointLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
		}
		pointLights[i] = std::move(pointLight);
		turnPointLightOn(i);
	}

	std::vector<PointLightsUniform>& pointLightUniform = pointLightsUniformBuffer.getData();
	pointLightUniform.clear();
	pointLightUniform.emplace_back();
	PointLightsUniform& newElement = pointLightUniform.back();
	newElement.activeLightMask = activePointLightsMask;
	newElement.pointLights = std::move(pointLights);

	/**
	* Vulkan은 데이터를 넘길 때, 배열인지 단일 데이터인지를 명시하지 않습니다.
	* 대신, 쉐이더가 선언한 UBO 구조체에 따라 데이터를 해석합니다.
	* 따라서 쉐이더가 UBO 배열로 선언되어 있지 않으면 제대로 동작하지 않을 수 있습니다.
	*/

	pointLightsUniformBuffer.CopyData(currentImage);
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

	//	colorUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	dirLightUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	pointLightsUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	position
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	normal
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	color + specular 
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
	createPipelineLayout(lightingPass.descriptorSetLayout, lightingPass.pipelineLayout);
	
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
	auto vertShaderCode = readFile("shaders/shadervert.spv");
	auto fragShaderCode = readFile("shaders/ForwardPassfrag.spv");

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	Vertex::getBindingDescriptions(bindingDescriptions);

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	Vertex::getAttributeDescriptions(attributeDescriptions);

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

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

	vertShaderCode = readFile("shaders/ObjectShadervert.spv");
	fragShaderCode = readFile("shaders/ObjectShaderfrag.spv");

	vertShaderModule = createShaderModule(vertShaderCode);
	fragShaderModule = createShaderModule(fragShaderCode);

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

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineObject) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline");
	}

	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, fragShaderModule, nullptr);

	/**
	* For lightpass
	*/
	vertShaderCode = readFile("shaders/LightingPassvert.spv");
	fragShaderCode = readFile("shaders/LightingPassfrag.spv");

	vertShaderModule = createShaderModule(vertShaderCode);
	fragShaderModule = createShaderModule(fragShaderCode);

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

//void VulkanTutorialExtension::createPointLightsGraphicsPipeline()
//{
//	auto vertShaderCode = readFile("shaders/shadervert.spv");
//	auto fragShaderCode = readFile("shaders/shaderfrag.spv");
//
//	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
//	Vertex::getBindingDescriptions(bindingDescriptions);
//
//	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
//	Vertex::getAttributeDescriptions(attributeDescriptions);
//
//	createGraphicsPipeline(vertShaderCode, fragShaderCode, pipelineLayoutPointLights, bindingDescriptions, attributeDescriptions, graphicsPipelinePointLights);
//}

//void VulkanTutorialExtension::createObjectGraphicsPipelines()
//{
//	auto objectShaderVertCode = readFile("shaders/ObjectShadervert.spv");
//	auto objectShaderFragCode = readFile("shaders/ObjectShaderfrag.spv");
//
//	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
//	Vertex::getBindingDescriptions(bindingDescriptions);
//	Instance::getBindingDescriptions(bindingDescriptions);
//
//	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
//	Vertex::getAttributeDescriptions(attributeDescriptions);
//	Instance::getAttributeDescriptions(attributeDescriptions);
//
//	createGraphicsPipeline(objectShaderVertCode, objectShaderFragCode, pipelineLayoutObject, bindingDescriptions, attributeDescriptions, graphicsPipelineObject);
//}

//void VulkanTutorialExtension::createLightingPassGraphicsPipelines()
//{
//	auto ShaderFragCode = readFile("shaders/LightingPassfrag.spv");
//
//	createGraphicsPipeline(std::vector<char>(), ShaderFragCode, lightingPassPipelineLayout, std::vector<VkVertexInputBindingDescription>(), std::vector<VkVertexInputAttributeDescription>(), lightingPassPipeline);
//}

void VulkanTutorialExtension::recordRenderPassCommands(VkCommandBuffer commandBuffer, size_t i)
{
	VulkanTutorial::recordRenderPassCommands(commandBuffer, i);

	for (const RenderObject& r : mainDrawContext.OpaqueSurfaces)
	{
		drawRenderObject(commandBuffer, i, r);
	}

	//VkBuffer vertexBuffers[]{ vertexBuffer };
	//VkDeviceSize offsets[]{ 0 };

	//vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
	//vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	//VkBuffer instanceBuffersToBind[]{ instanceBuffers[usingInstanceBufferIndex] };
	//vkCmdBindVertexBuffers(commandBuffers[i], 1, 1, instanceBuffersToBind, offsets);

	// Q : vkCmdBindVertexBuffers() persistent between pipeline changes?
	// A : They are persistent. Binding a new pipeline will only reset the static state and if the pipeline has dynamic state then it will reset the dynamic state as well.
	
	// Objects
	//vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineObject);
	//vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutObject, 0, 1, &descriptorSetsObject[i], 0, nullptr);
	//vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), static_cast<uint32_t>(instanceCount), 0, 0, 0);
}

void VulkanTutorialExtension::recordForwardPassCommands(VkCommandBuffer commandBuffer, size_t i)
{
	VulkanTutorial::recordForwardPassCommands(commandBuffer, i);

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
	//pushConstants.vertexBuffer = draw.vertexBufferAddress;
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

	//createVertexBuffer(cube.vertices, cube.vertexBuffer, cube.vertexBufferMemory);
	//createIndexBuffer(cube.indices, cube.indexBuffer, cube.indexBufferMemory);

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