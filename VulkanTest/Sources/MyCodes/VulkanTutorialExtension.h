#pragma once

#include "VulkanTutorial.h"

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

	static void mouseCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto app = reinterpret_cast<VulkanTutorial*>(glfwGetWindowUserPointer(window));
		app->getCamera().ProcessMouseMovement(xpos, ypos);
	}

	void createDescriptorSetsLight(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetsObject(std::vector<VkDescriptorSet>& outDescriptorSets);

private:
	UniformBuffer<Transform> lightTransformUniformBuffer;
	VkPipeline graphicsPipelineObject;
	std::vector<VkDescriptorSet> descriptorSetsObject;

	double deltaTime = 0.0f; // Time between current frame and last frame
	double lastFrame = 0.0f; // Time of last frame
};