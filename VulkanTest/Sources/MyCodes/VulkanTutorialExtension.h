#pragma once

#include "VulkanTutorial.h"
#include "UniformBufferTypes.h"

/**
* https://vulkan-tutorial.com/ 에서 진행한 튜토리얼 프로젝트는 VulkanTutorial 클래스에 있다. 
* 이 프로젝트를 토대로 개인적으로 추가한 코드는 최대한 VulkanTutorialExtension에 구현해 분리한다.
*/

// 인스턴스 개수를 바꿀 때 이미 사용중인 버퍼를 삭제하지 않게 하기 위해 더블버퍼링을 사용한다.
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
	*	Vulkan-Tutorial에서 추가적인 기능을 구현하고 싶은 함수들을 분리해 구현한다. 
	*	VulkanTutorial::initVulkan()의 순서는 그대로 유지해야 한다.
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
	*	GUI와 관련된 함수들.
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
	// 포인트라이트용
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

	// 오브젝트용
	VkPipeline graphicsPipelineObject;
	VkPipelineLayout pipelineLayoutObject;
	std::vector<VkDescriptorSet> descriptorSetsObject;

	Camera camera;

	/**
	*	GUI와 관련된 변수들
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