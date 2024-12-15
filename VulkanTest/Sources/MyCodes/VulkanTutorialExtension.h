#pragma once

#include "VulkanTutorial.h"

/**
* https://vulkan-tutorial.com/ 에서 진행한 튜토리얼 프로젝트는 VulkanTutorial 클래스에 있다. 
* 이 프로젝트를 토대로 개인적으로 추가한 코드는 최대한 VulkanTutorialExtension에 구현해 분리한다.
*/

//  ObjectShader.frag 안의 NR_POINT_LIGHTS와 반드시 일치시킨다.
#define NR_POINT_LIGHTS 1

class VulkanTutorialExtension : public VulkanTutorial{
public:

	VulkanTutorialExtension();

	static bool useDirectionalLight;
	static bool usePointLights;
	static float pointLightlinear;
	static float pointLightQuadratic;
private:

	/**
	*	Vulkan-Tutorial에서 추가적인 기능을 구현하고 싶은 함수들을 분리해 구현한다. 
	*	VulkanTutorial::initVulkan()의 순서는 그대로 유지해야 한다.
	*/
	void initWindow() override;
	void initVulkan() override;
	void processInput() override;
	void createUniformBuffers() override;
	void createDescriptorPool() override;
	void createDescriptorSets() override;
	void updateUniformBuffer(uint32_t currentImage) override;
	void clearUniformBuffer(uint32_t i) override;
	void createDescriptorSetLayouts() override;
	void createGraphicsPipelines() override;
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

	void createObjectGraphicsPipelines();
	void createPointLightsGraphicsPipeline();
	void createDescriptorSetsPointLights(UniformBuffer<Transform>& inUniformBuffer, std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetsObject(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetLayoutsForObjects();
	void createDescriptorSetLayoutsForPointLights();
	void createInstanceBuffer();
	void setWindowFocused(int inFocused);

	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
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

private:
	std::vector<glm::mat4> instances;
	VkBuffer instanceBuffer;
	VkDeviceMemory instanceBufferMemory;

	std::array<UniformBuffer<Transform>, NR_POINT_LIGHTS> lightTransformUniformBuffer;
	UniformBuffer<Transform> objectTransformUniformBuffer;
	UniformBuffer<ColorUBO> colorUniformBuffer;
	UniformBuffer<Material> materialUniformBuffer;
	UniformBuffer<DirLight> dirLightUniformBuffer;
	UniformBuffer<PointLight> pointLightsUniformBuffer;

	// 오브젝트용
	VkPipeline graphicsPipelineObject;
	VkPipelineLayout pipelineLayoutObject;
	std::vector<VkDescriptorSet> descriptorSetsObject;

	// 포인트라이트용
	VkPipeline graphicsPipelinePointLights;
	VkPipelineLayout pipelineLayoutPointLights;
	VkDescriptorSetLayout descriptorSetLayoutPointLights;
	std::array<std::vector<VkDescriptorSet>, NR_POINT_LIGHTS> descriptorSetsPointLights;
	
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