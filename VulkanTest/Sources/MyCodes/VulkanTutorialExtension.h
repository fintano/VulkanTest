#pragma once

#include "VulkanTutorial.h"

/**
* https://vulkan-tutorial.com/ ���� ������ Ʃ�丮�� ������Ʈ�� VulkanTutorial Ŭ������ �ִ�. 
* �� ������Ʈ�� ���� ���������� �߰��� �ڵ�� �ִ��� VulkanTutorialExtension�� ������ �и��Ѵ�.
*/

class VulkanTutorialExtension : public VulkanTutorial{
public:

	VulkanTutorialExtension();

private:

	void initWindow() override;
	void initVulkan() override;
	void processInput() override;
	void createUniformBuffers() override;
	void createDescriptorSets() override;
	void updateUniformBuffer(uint32_t currentImage) override;
	void clearUniformBuffer(uint32_t i) override;
	void createGraphicsPipelines() override;
	void createDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings) override;
	void RecordRenderPassCommands(VkCommandBuffer commandBuffer, size_t index) override;
	void cleanUpSwapchain() override;
	void loadModel() override;
	void createBuffers() override;

	static void mouseCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto app = reinterpret_cast<VulkanTutorialExtension*>(glfwGetWindowUserPointer(window));
		app->camera.ProcessMouseMovement(xpos, ypos);
	}

	void createDescriptorSetsLight(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetsObject(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createInstanceBuffer();

private:
	std::vector<glm::mat4> instances;
	VkBuffer instanceBuffer;
	VkDeviceMemory instanceBufferMemory;

	UniformBuffer<Transform> lightTransformUniformBuffer;
	UniformBuffer<Transform> objectTransformUniformBuffer;
	UniformBuffer<ColorUBO> colorUniformBuffer;
	UniformBuffer<Material> materialUniformBuffer;
	UniformBuffer<Light> lightUniformBuffer;

	VkPipeline graphicsPipelineObject;
	std::vector<VkDescriptorSet> descriptorSetsObject;
	
	Camera camera;

	double deltaTime = 0.0f; // Time between current frame and last frame
	double lastFrame = 0.0f; // Time of last frame
};