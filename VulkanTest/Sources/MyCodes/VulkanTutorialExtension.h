#pragma once

#include "VulkanTutorial.h"
#include "UniformBufferTypes.h"

/**
* https://vulkan-tutorial.com/ ���� ������ Ʃ�丮�� ������Ʈ�� VulkanTutorial Ŭ������ �ִ�. 
* �� ������Ʈ�� ���� ���������� �߰��� �ڵ�� �ִ��� VulkanTutorialExtension�� ������ �и��Ѵ�.
*/

// �ν��Ͻ� ������ �ٲ� �� �̹� ������� ���۸� �������� �ʰ� �ϱ� ���� ������۸��� ����Ѵ�.
#define INSTANCE_BUFFER_COUNT 2

class VulkanTutorialExtension : public VulkanTutorial{
public:

	VulkanTutorialExtension();

	static int instanceCount;
	static int maxInstanceCount;
	static bool useDirectionalLight;
	static bool debugGBuffers;
	static bool usePointLights;
	static std::array<bool, NR_POINT_LIGHTS> pointLightsSwitch;
	static float pointLightlinear;
	static float pointLightQuadratic;
private:

	/**
	*	Vulkan-Tutorial���� �߰����� ����� �����ϰ� ���� �Լ����� �и��� �����Ѵ�. 
	*	VulkanTutorial::initVulkan()�� ������ �״�� �����ؾ� �Ѵ�.
	*/
	void initVulkan() override;
	void processInput() override;
	void initWindow() override;
	void createUniformBuffers() override;
	void createDescriptorPool() override;
	void createDescriptorSets() override;
	void updateUniformBuffer(uint32_t currentImage) override;
	void clearUniformBuffer(uint32_t i) override;
	void createDescriptorSetLayouts() override;
	void createGraphicsPipelines() override;
	void recordRenderPassCommands(VkCommandBuffer commandBuffer, size_t index) override;
	void recordForwardPassCommands(VkCommandBuffer commandBuffer, size_t index) override;
	void loadModels() override;
	void createBuffers() override;
	void recreateSwapChain() override;
	void preDrawFrame(uint32_t imageIndex) override;
	void drawFrame() override;
	void postDrawFrame(uint32_t imageIndex) override;
	void createCommandPool() override;
	void createCommandBuffers() override;
	void createFrameBuffers() override;
	void createRenderPass() override;
	void cleanUpSwapchain() override;
	void cleanUp() override;
	
	//void createObjectGraphicsPipelines();
	//void createLightingPassGraphicsPipelines();
	//void createPointLightsGraphicsPipeline();
	void createDescriptorSetsPointLights(UniformBuffer<Transform>& inUniformBuffer, std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetsObject(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createLightingPassDescriptorSets(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetLayoutsForObjects();
	void createLightingPassDescriptorSetLayout();
	void createDescriptorSetLayoutsForPointLights();
	void createInstanceBuffer(uint32_t imageIndex);
	void setWindowFocused(int inFocused);
	bool instanceCountChanged();
	void recreateInstanceBuffer(uint32_t imageIndex);
	void clearPointLightsSwitch();
	void turnPointLightOn(int index);
	bool isLightOn(int index);
	bool pointLightSwitchChanged(uint32_t index);

	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void mouseScrollCallback(GLFWwindow* window, double, double yoffset);
	static void focusCallback(GLFWwindow* window, int focused);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	/**
	*	GUI�� ���õ� �Լ���.
	*/
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

public:
	// ����Ʈ����Ʈ��
	VkPipeline graphicsPipelinePointLights;
	VkPipelineLayout pipelineLayoutPointLights;
	VkDescriptorSetLayout descriptorSetLayoutPointLights;
	std::array<std::vector<VkDescriptorSet>, NR_POINT_LIGHTS> descriptorSetsPointLights;
	std::vector<int> previousActivePointLightsMask;
	int activePointLightsMask = 0;

	int getSwapchainImageNum() { return swapChainImages.size(); }
	VkExtent2D getSwapchainExtent() { return swapChainExtent; }

private:
	Model cube;

	std::vector<Instance> instances;
	std::array<VkBuffer, INSTANCE_BUFFER_COUNT> instanceBuffers;
	std::array<VkDeviceMemory, INSTANCE_BUFFER_COUNT> instanceBufferMemories;
	int usingInstanceBufferIndex = 0;
	int previousInstanceCount = instanceCount;

	std::array<UniformBuffer<Transform>, NR_POINT_LIGHTS> lightTransformUniformBuffer;
	UniformBuffer<Transform> objectTransformUniformBuffer;
	UniformBuffer<ColorUBO> colorUniformBuffer;
	UniformBuffer<Material> materialUniformBuffer;
	UniformBuffer<DirLight> dirLightUniformBuffer;
	UniformBuffer<PointLightsUniform> pointLightsUniformBuffer;

	// ������Ʈ��
	VkPipeline graphicsPipelineObject;
	VkPipelineLayout pipelineLayoutObject;
	std::vector<VkDescriptorSet> descriptorSetsObject;

	Camera camera;

	/**
	*	GUI�� ���õ� ������
	*/
	VkDescriptorPool imGuiDescriptorPool;
	VkRenderPass imGuiRenderPass;
	VkCommandPool imGuiCommandPool;
	std::vector<VkCommandBuffer> imGuiCommandBuffers;
	std::vector<VkFramebuffer> imGuiFrameBuffers;

	bool focused;

	double deltaTime = 0.0f; // Time between current frame and last frame
	double lastFrame = 0.0f; // Time of last frame
};