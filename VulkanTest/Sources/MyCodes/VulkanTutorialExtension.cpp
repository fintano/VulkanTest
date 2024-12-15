#include "VulkanTutorialExtension.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "VulkanTutorialExtensionImGui.h"

/**
* GUI에서 사용할 변수들을 모아놓는다. 
*/
bool VulkanTutorialExtension::useDirectionalLight = false;
bool VulkanTutorialExtension::usePointLights = false;
float VulkanTutorialExtension::pointLightlinear = 0.09f;
float VulkanTutorialExtension::pointLightQuadratic = 0.032f;

VulkanTutorialExtension::VulkanTutorialExtension()
	: camera({ 5.f, 5.f, 5.f }, { 0.f,1.f,0.f })
{
}

void VulkanTutorialExtension::initWindow()
{
	VulkanTutorial::initWindow();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetWindowFocusCallback(window, focusCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	initImGui();
}

void VulkanTutorialExtension::initVulkan()
{
	VulkanTutorial::initVulkan();

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

	ImGui::stringToDebug.append(camera.ToString());
}

void VulkanTutorialExtension::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	bool bLeftMouseButtonClicked = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

	if (bLeftMouseButtonClicked)
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

void VulkanTutorialExtension::focusCallback(GLFWwindow* window, int focused)
{
	auto app = reinterpret_cast<VulkanTutorialExtension*>(glfwGetWindowUserPointer(window));
	assert(app);

	app->setWindowFocused(focused);
}

void VulkanTutorialExtension::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) 
	{
		auto app = reinterpret_cast<VulkanTutorialExtension*>(glfwGetWindowUserPointer(window));
		assert(app);

		app->camera.ResetPreoffsets();
	}
}

void VulkanTutorialExtension::createUniformBuffers()
{
	VulkanTutorial::createUniformBuffers();

	objectTransformUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	colorUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	materialUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	dirLightUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	pointLightsUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice, NR_POINT_LIGHTS);
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

	for (int lightIndex = 0; lightIndex < NR_POINT_LIGHTS; lightIndex++)
	{
		createDescriptorSetsPointLights(lightTransformUniformBuffer[lightIndex], descriptorSetsPointLights[lightIndex]);
	}
	createDescriptorSetsObject(descriptorSetsObject);
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

	/** PointLights Object */
	for (int lightIndex = 0; lightIndex < NR_POINT_LIGHTS; lightIndex++)
	{
		std::vector<Transform> pointLightTransforms{};
		Transform transform;
		transform.model = glm::translate(glm::mat4(1.f), pointLightPositions[lightIndex]);
		transform.model = glm::scale(transform.model, glm::vec3(0.2f));
		transform.view = viewMat;
		transform.proj = persMat;
		pointLightTransforms.emplace_back(std::move(transform));

		lightTransformUniformBuffer[lightIndex].CopyData(currentImage, pointLightTransforms);
	}

	// Object
	Transform ubo{};
	ubo.model = glm::mat4(1.f);
	ubo.view = viewMat;
	ubo.proj = persMat;

	objectTransformUniformBuffer.CopyData(currentImage, { ubo });

	ColorUBO colorUbo;
	colorUbo.objectColor = glm::vec3(1.0f, 0.5f, 0.31f);
	colorUbo.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	colorUbo.viewPos = camera.Position;

	colorUniformBuffer.CopyData(currentImage, { colorUbo });

	// Material 
	Material material;
	material.ambient = glm::vec3(1.0f, 0.5f, 0.31f);
	material.diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
	material.specular = glm::vec3(1.0f, 0.5f, 0.31f);
	// 유니폼 버퍼 멤버의 타입을 정할 때 float은 잘 안된다. 
	// float을 할거면 vec3/vec4로 해서 컴포넌트로 넘겨주는 것만 작동한다. 
	material.shininess = glm::vec3(32.0f, 0.5f, 0.31f);

	materialUniformBuffer.CopyData(currentImage, { material });

	// Directional Light;
	DirLight dirLight;
	if (useDirectionalLight)
	{
		dirLight.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
		dirLight.diffuse = glm::vec3(0.4f, 0.4f, 0.4f); // darken diffuse light a bit
		dirLight.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	}
	dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);

	dirLightUniformBuffer.CopyData(currentImage, { dirLight });

	// Point Lights
	std::vector<PointLight> pointLights;
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
		pointLights.emplace_back(std::move(pointLight));
	}

	/**
	* Vulkan은 데이터를 넘길 때, 배열인지 단일 데이터인지를 명시하지 않습니다.
	* 대신, 쉐이더가 선언한 UBO 구조체에 따라 데이터를 해석합니다.
	* 따라서 쉐이더가 UBO 배열로 선언되어 있지 않으면 제대로 동작하지 않을 수 있습니다.
	*/	

	pointLightsUniformBuffer.CopyData(currentImage, pointLights);
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
}

void VulkanTutorialExtension::createDescriptorSetLayouts()
{
	VulkanTutorial::createDescriptorSetLayouts();

	createDescriptorSetLayoutsForPointLights();
	createDescriptorSetLayoutsForObjects();
}

void VulkanTutorialExtension::createDescriptorSetLayoutsForPointLights()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	//	Transform
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, bindings);

	createDescriptorSetLayout(bindings, descriptorSetLayoutPointLights);
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

	createDescriptorSetLayout(bindings, descriptorSetLayout);
}

void VulkanTutorialExtension::createGraphicsPipelines()
{
	VulkanTutorial::createGraphicsPipelines();

	createPipelineLayout(descriptorSetLayoutPointLights, pipelineLayoutPointLights);
	createPipelineLayout(descriptorSetLayout, pipelineLayoutObject);
	createPointLightsGraphicsPipeline();
	createObjectGraphicsPipelines();
}

void VulkanTutorialExtension::createPointLightsGraphicsPipeline()
{
	auto vertShaderCode = readFile("shaders/shadervert.spv");
	auto fragShaderCode = readFile("shaders/shaderfrag.spv");
	createGraphicsPipeline(vertShaderCode, fragShaderCode, pipelineLayoutPointLights, graphicsPipelinePointLights);
}

void VulkanTutorialExtension::createObjectGraphicsPipelines()
{
	auto objectShaderVertCode = readFile("shaders/ObjectShadervert.spv");
	auto objectShaderFragCode = readFile("shaders/ObjectShaderfrag.spv");
	createGraphicsPipeline(objectShaderVertCode, objectShaderFragCode, pipelineLayoutObject, graphicsPipelineObject);
}

void VulkanTutorialExtension::RecordRenderPassCommands(VkCommandBuffer commandBuffer, size_t i)
{
	VulkanTutorial::RecordRenderPassCommands(commandBuffer, i);

	VkBuffer vertexBuffers[]{ vertexBuffer };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	VkBuffer instanceBuffers[]{ instanceBuffer };
	vkCmdBindVertexBuffers(commandBuffers[i], 1, 1, instanceBuffers, offsets);

	// Point Lights
	vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelinePointLights);
	for (int lightIndex = 0; lightIndex < NR_POINT_LIGHTS; lightIndex++)
	{
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutPointLights, 0, 1, &descriptorSetsPointLights[lightIndex][i], 0, nullptr);
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}

	// Q : vkCmdBindVertexBuffers() persistent between pipeline changes?
	// A : They are persistent. Binding a new pipeline will only reset the static state and if the pipeline has dynamic state then it will reset the dynamic state as well.
	
	// Objects
	vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineObject);
	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayoutObject, 0, 1, &descriptorSetsObject[i], 0, nullptr);
	vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), static_cast<uint32_t>(instances.size()), 0, 0, 0);

}

void VulkanTutorialExtension::createInstanceBuffer()
{
	VkDeviceSize bufferSize = sizeof(instances[0]) * instances.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, instances.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, instanceBuffer, instanceBufferMemory);
	copyBuffer(stagingBuffer, instanceBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanTutorialExtension::setWindowFocused(int inFocused)
{
	focused = inFocused;
}

void VulkanTutorialExtension::loadModel()
{
	VulkanTutorial::loadModel();

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

	for (unsigned int i = 0; i < instanceCount; i++)
	{
		glm::mat4 identity = glm::mat4(1.0f);
		// the obj model file is rotated incorrectly, so it needs to be fixed.
		glm::mat4 xRotation = glm::rotate(identity, glm::radians(270.f), glm::vec3(1.f, 0.f, 0.f));
		glm::mat4 yRotation = glm::rotate(identity, glm::radians(270.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 model = yRotation * xRotation;

		model = glm::translate(model, cubePositions[i]);
		float angle = 20.0f * i;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

		instances.push_back(model);
	}
}

void VulkanTutorialExtension::createBuffers()
{
	VulkanTutorial::createBuffers();

	createInstanceBuffer();
}

void VulkanTutorialExtension::recreateSwapChain()
{
	VulkanTutorial::recreateSwapChain();

	ImGui_ImplVulkan_SetMinImageCount(swapChainImages.size());
}

void VulkanTutorialExtension::preDrawFrame(uint32_t imageIndex)
{
	VulkanTutorial::preDrawFrame(imageIndex);

	drawImGui(imageIndex);
}

void VulkanTutorialExtension::drawFrame()
{
	VulkanTutorial::drawFrame();
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

	vkDestroyBuffer(device, instanceBuffer, nullptr);
	vkFreeMemory(device, instanceBufferMemory, nullptr);
	vkDestroyDescriptorSetLayout(device, descriptorSetLayoutPointLights, nullptr);
	VulkanTutorial::cleanUp();
}