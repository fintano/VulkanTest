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
	void createDescriptorPool() override;
	void createDescriptorSets() override;
	void updateUniformBuffer(uint32_t currentImage) override;
	void clearUniformBuffer(uint32_t i) override;
	void createGraphicsPipelines() override;
	void createDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings) override;
	void RecordRenderPassCommands(VkCommandBuffer commandBuffer, size_t index) override;
	void loadModel() override;
	void createBuffers() override;
	void recreateSwapChain() override;
	void preDrawFrame(uint32_t imageIndex) override;
	void drawFrame() override;
	void createCommandPool() override;
	void createCommandBuffers() override;
	void createFrameBuffers() override;
	void createRenderPass() override;
	void cleanUpSwapchain() override;
	void cleanUp() override;

	void createDescriptorSetsLight(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetsObject(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createInstanceBuffer();
	void setWindowFocused(int inFocused);

	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void focusCallback(GLFWwindow* window, int focused);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	void initImGui();
	void createImGui();
	void createImGuiDescriptorPool();
	void drawImGui(uint32_t imageIndex);
	void createImGuiCommandPool();
	void createImGuiCommandBuffers();
	void createImGuiFrameBuffers();
	void createImGuiRenderPass();
	void cleanUpImGuiSwapchain();
	void cleanUpImGui();

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

	bool focused;

	double deltaTime = 0.0f; // Time between current frame and last frame
	double lastFrame = 0.0f; // Time of last frame
};