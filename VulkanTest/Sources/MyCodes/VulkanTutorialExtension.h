#pragma once

#include "VulkanTutorial.h"

/**
* https://vulkan-tutorial.com/ 에서 진행한 튜토리얼 프로젝트는 VulkanTutorial 클래스에 있다. 
* 이 프로젝트를 토대로 개인적으로 추가한 코드는 최대한 VulkanTutorialExtension에 구현해 분리한다.
*/

class VulkanTutorialExtension : public VulkanTutorial{
public:

	VulkanTutorialExtension();

private:

	void initWindow() override;
	void initVulkan() override;
	void processInput() override;
	void createUniformBuffers() override;
	void createDescriptorPool() override;
	void createDescriptorSets() override;
	void updateUniformBuffer(uint32_t currentImage) override;
	void clearUniformBuffer(uint32_t i) override;
	void createGraphicsPipelines() override;
	void createDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings) override;
	void RecordRenderPassCommands(VkCommandBuffer commandBuffer, size_t index) override;
	void cleanUpSwapchain() override;
	void loadModel() override;
	void createBuffers() override;
	void recreateSwapChain() override;
	void preDrawFrame(uint32_t imageIndex) override;
	void drawFrame() override;
	void createCommandPool() override;
	void createCommandBuffers() override;
	void createFrameBuffers() override;

	static void mouseCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto app = reinterpret_cast<VulkanTutorialExtension*>(glfwGetWindowUserPointer(window));
		app->camera.ProcessMouseMovement(xpos, ypos);
	}

	void createImGuiRenderPass();
	void createImGui();
	void loadFonts();
	void createDescriptorSetsLight(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetsObject(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createInstanceBuffer();

private:
	std::vector<glm::mat4> instances;
	VkBuffer instanceBuffer;
	VkDeviceMemory instanceBufferMemory;
	VkDescriptorPool imGuiDescriptorPool;
	VkRenderPass imGuiRenderPass;
	VkCommandPool imGuiCommandPool;
	std::vector<VkCommandBuffer> imGuiCommandBuffers;
	std::vector<VkFramebuffer> imGuiFrameBuffers;

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