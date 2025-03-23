#pragma once

#include "VulkanTutorial.h"
#include "UniformBufferTypes.h"

class IrradianceCubeMap;
class Skybox;
struct Cube;

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
	static int debugDisplayTarget;
	static bool usePointLights;
	static std::array<bool, NR_POINT_LIGHTS> pointLightsSwitch;
	static float pointLightlinear;
	static float pointLightQuadratic;
	static float pointLightIntensity; 
	static float directionalLightIntensity;
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
	void recordCommandBuffer(VkCommandBuffer commandBuffer, size_t index) override;
	void recordRenderPassCommands(VkCommandBuffer commandBuffer, size_t index) override;
	void recordLightingRenderPassCommands(VkCommandBuffer commandBuffer, size_t index) override;
	void recordForwardPassCommands(VkCommandBuffer commandBuffer, size_t index) override;
	void loadModels() override;
	void createBuffers() override;
	void recreateSwapChain() override;
	void preDrawFrame(uint32_t imageIndex) override;
	void drawFrame() override;
	void postDrawFrame(uint32_t imageIndex) override;
	void createCommandPool() override;
	void onPostInitVulkan() override;
	void createCommandBuffers() override;
	void createFrameBuffers() override;
	void createRenderPass() override;
	void cleanUpSwapchain() override;
	void cleanUp() override;
	
	//void createObjectGraphicsPipelines();
	//void createLightingPassGraphicsPipelines();
	//void createPointLightsGraphicsPipeline();
	void createGlobalDescriptorSets();
	void createDescriptorSetsPointLights(UniformBuffer<Transform>& inUniformBuffer, std::vector<VkDescriptorSet>& outDescriptorSets);
	void createDescriptorSetsObject(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createLightingPassDescriptorSets(std::vector<VkDescriptorSet>& outDescriptorSets);
	void createGlobalDescriptorSetLayout();
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

	int getSwapchainImageNum() { return static_cast<int>(swapChainImages.size()); }
	VkExtent2D getSwapchainExtent() { return swapChainExtent; }
	virtual VkDescriptorSetLayout getGlobalDescriptorSetLayout() override { return globalDescriptorSetLayout; }
	AllocatedImage getDefaultTexture2D() { return whiteTexture; }
	VkSampler getDefaultTextureSampler() { return textureSampler; }
	void drawRenderObject(VkCommandBuffer commandBuffer, size_t i, const RenderObject& draw);

private:
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
	
public:
	std::shared_ptr<IrradianceCubeMap> irradianceCubeMap;
	std::shared_ptr<Skybox> skybox;
	std::shared_ptr<Cube> cube;
	/** material system */
	//std::vector<std::shared_ptr<struct MeshAsset>> testMeshes;
	DrawContext mainDrawContext;
	//std::unordered_map<std::string, std::shared_ptr<Node>> loadedNodes;
	std::unordered_map<std::string, std::shared_ptr<LoadedGLTF>> loadedScenes;

	/** ���͸��� */
	GLTFMaterial defaultData;
	GLTFMetallic_Roughness metalRoughMaterial;
	// �ٲ��� �����Ƿ� ���� �����Ӹ��� �ٸ� ������ ���۸� ���� ���� ������ ����.
	UniformBuffer<GLTFMetallic_Roughness::MaterialConstants> materialConstants;

	/** �۷ι� ������ */
	UniformBuffer<GPUSceneData> globalSceneData;
	VkDescriptorSet globalDescriptorSet;
	VkDescriptorSetLayout globalDescriptorSetLayout;

	void init_default_data();
	void update_scene(uint32_t currentImage);
};