#include "VulkanTutorialExtension.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

VulkanTutorialExtension::VulkanTutorialExtension()
	: camera({ 0.f, 0.f, -5.f }, { 0.f,1.f,0.f })
{
}

void VulkanTutorialExtension::initWindow()
{
	VulkanTutorial::initWindow();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetWindowFocusCallback(window, focusCallback);

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
			// ImGui가 마우스 입력을 처리 중이므로 GLFW의 입력 처리를 무시합니다.
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

void VulkanTutorialExtension::createUniformBuffers()
{
	VulkanTutorial::createUniformBuffers();

	lightTransformUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	objectTransformUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	colorUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	materialUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
	lightUniformBuffer.createUniformBuffer(swapChainImages.size(), device, physicalDevice);
}

void VulkanTutorialExtension::createDescriptorPool()
{
	VulkanTutorial::createDescriptorPool();

	createImGuiDescriptorPool();
}

void VulkanTutorialExtension::createDescriptorSets()
{
	VulkanTutorial::createDescriptorSets();

	//createDescriptorSetsLight(descriptorSets);
	createDescriptorSetsObject(descriptorSetsObject);
}

void VulkanTutorialExtension::updateUniformBuffer(uint32_t currentImage)
{
	VulkanTutorial::updateUniformBuffer(currentImage);

	glm::mat4 viewMat = camera.GetViewMatrix();
	glm::mat4 persMat = glm::perspective(glm::radians(45.f), swapChainExtent.width / (float)(swapChainExtent.height), 0.1f, 100.f);
	persMat[1][1] *= -1;

	/** Light Source */

	const glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

	Transform ubo{};
	ubo.model = glm::translate(glm::mat4(1.f), lightPos);
	ubo.model = glm::scale(ubo.model, glm::vec3(0.2f));
	ubo.view = viewMat;
	ubo.proj = persMat;

	lightTransformUniformBuffer.CopyData(currentImage, ubo);

	/** Object */

	ubo.model = glm::mat4(1.f);
	ubo.view = viewMat;
	ubo.proj = persMat;

	objectTransformUniformBuffer.CopyData(currentImage, ubo);

	ColorUBO colorUbo;
	colorUbo.objectColor = glm::vec3(1.0f, 0.5f, 0.31f);
	colorUbo.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	colorUbo.viewPos = camera.Position;

	colorUniformBuffer.CopyData(currentImage, colorUbo);

	Material material;
	material.ambient = glm::vec3(1.0f, 0.5f, 0.31f);
	material.diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
	material.specular = glm::vec3(1.0f, 0.5f, 0.31f);
	// 유니폼 버퍼 멤버의 타입을 정할 때 float은 잘 안된다. 
	// float을 할거면 vec3/vec4로 해서 컴포넌트로 넘겨주는 것만 작동한다. 
	material.shininess = glm::vec3(32.0f, 0.5f, 0.31f);

	materialUniformBuffer.CopyData(currentImage, material);

	glm::vec3 lightColor;
	lightColor.x = sin(glfwGetTime() * 2.0f);
	lightColor.y = sin(glfwGetTime() * 0.7f);
	lightColor.z = sin(glfwGetTime() * 1.3f);

	glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
	glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

	Light light;
	light.ambient = ambientColor;//glm::vec3(0.2f, 0.2f, 0.2f);
	light.diffuse = diffuseColor;//glm::vec3(0.5f, 0.5f, 0.5f); // darken diffuse light a bit
	light.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	light.direction = glm::vec3(-0.2f, -1.0f, -0.3f);

	lightUniformBuffer.CopyData(currentImage, light);
}

void VulkanTutorialExtension::clearUniformBuffer(uint32_t i)
{
	VulkanTutorial::clearUniformBuffer(i);

	lightTransformUniformBuffer.destroy(i);
	objectTransformUniformBuffer.destroy(i);
	colorUniformBuffer.destroy(i);
	materialUniformBuffer.destroy(i);
	lightUniformBuffer.destroy(i);
}

void VulkanTutorialExtension::createGraphicsPipelines()
{
	VulkanTutorial::createGraphicsPipelines();

	auto objectShaderVertCode = readFile("shaders/ObjectShadervert.spv");
	auto objectShaderFragCode = readFile("shaders/ObjectShaderfrag.spv");
	createGraphicsPipeline(objectShaderVertCode, objectShaderFragCode, graphicsPipelineObject);
}

void VulkanTutorialExtension::createDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
	VulkanTutorial::createDescriptorSetLayoutBindings(bindings);

	//	Transform
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, bindings);
	//	TexSampler
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	colorUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	materialUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
	//	lightUniformBuffer
	createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, bindings);
}

void VulkanTutorialExtension::RecordRenderPassCommands(VkCommandBuffer commandBuffer, size_t i)
{
	VulkanTutorial::RecordRenderPassCommands(commandBuffer, i);

	// Light
	//vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	VkBuffer vertexBuffers[]{ vertexBuffer };
	VkDeviceSize offsets[]{ 0 };
	vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

	VkBuffer instanceBuffers[]{ instanceBuffer };
	vkCmdBindVertexBuffers(commandBuffers[i], 1, 1, instanceBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	//vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
	//vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	// Objects
	// 
	// Q : vkCmdBindVertexBuffers() persistent between pipeline changes?
	// A : They are persistent. Binding a new pipeline will only reset the static state and if the pipeline has dynamic state then it will reset the dynamic state as well.
	vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineObject);
	vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSetsObject[i], 0, nullptr);
	vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), static_cast<uint32_t>(instances.size()), 0, 0, 0);

}

void VulkanTutorialExtension::createDescriptorSetsLight(std::vector<VkDescriptorSet>& outDescriptorSets)
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
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = lightTransformUniformBuffer.uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Transform);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = outDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0; // layout (binding = 0)
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1; // how many descriptor you update
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		/** in vertex shader,
		 * layout(binding = 0) uniform Transform {
				mat4 model;
				mat4 view;
				mat4 proj;
			} ubo;
		 */

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = outDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1; // layout (binding = 1)
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1; // how many descriptor you update
		descriptorWrites[1].pImageInfo = &imageInfo;

		/** in fragment shader,
		 * layout(binding = 1) uniform sampler2D texSampler;
		 */

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
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		std::vector<VkWriteDescriptorSet> descriptorWrites;

		objectTransformUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);

		VkDescriptorImageInfo ImageInfo = CreateDescriptorImageInfo(textureImageView, textureSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		CreateWriteDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, outDescriptorSets[i], &ImageInfo, nullptr, descriptorWrites);

		colorUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);
		materialUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);
		lightUniformBuffer.createWriteDescriptorSet(i, outDescriptorSets[i], descriptorWrites);

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
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

	for (unsigned int i = 0; i < 10; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
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
	vkDestroyPipeline(device, graphicsPipelineObject, nullptr);

	cleanUpImGuiSwapchain();

	VulkanTutorial::cleanUpSwapchain();
}

void VulkanTutorialExtension::cleanUp()
{
	glfwSetWindowUserPointer(window, nullptr);

	cleanUpImGui();

	vkDestroyBuffer(device, instanceBuffer, nullptr);
	vkFreeMemory(device, instanceBufferMemory, nullptr);

	VulkanTutorial::cleanUp();
}