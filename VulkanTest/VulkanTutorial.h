#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <stdexcept>	
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>

#include "Camera.h"
#include "UniformBuffer.h"
#include "Vertex.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}


class VulkanTutorial {
public:

	VulkanTutorial();

	void run();
	Camera& getCamera() { return camera; }

protected:
	virtual void initWindow();
	virtual void initVulkan();
	virtual void processInput();
	virtual void createUniformBuffers();
	virtual void createDescriptorSets();
	virtual void updateUniformBuffer(uint32_t currentImage);
	virtual void clearUniformBuffer(uint32_t i);
	virtual void createGraphicsPipelines();
	virtual void createDescriptorSetLayoutBindings(std::vector<VkDescriptorSetLayoutBinding>& bindings);
	virtual void RecordRenderPassCommands(VkCommandBuffer commandBuffer, size_t index);
	virtual void cleanUpSwapchain();

	bool checkValidationLayerSupport();
	void createInstance();
	std::vector<const char*> getRequiredExtensions();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void setupSurface();
	void pickPhysicalDevice();
	VkSampleCountFlagBits getMaxUsableSampleCount();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice  device);
	void createLogicalDevice();
	void createSwapchain();
	void recreateSwapChain();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createImageViews();
	void createTextureImageView();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createTextureSampler();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createDescriptorSetLayoutBinding(VkDescriptorType Type, VkShaderStageFlags Stage, std::vector<VkDescriptorSetLayoutBinding>& bindings);
	void createGraphicsPipeline(const std::vector<char>& vertShaderCode, const std::vector<char>& fragShaderCode, VkPipeline& OutPipeline);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createFrameBuffers();
	void createCommandPool();
	void createColorResources();
	void createDepthResources();
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
	void createTextureImage();
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void loadModel();
	void createVertexBuffer();
	void createInstanceBuffer();
	void createIndexBuffer();
	void createDescriptorPool();
	VkDescriptorBufferInfo CreateDescriptorBufferInfo(VkBuffer& buffer, VkDeviceSize bufferSize);
	VkDescriptorImageInfo CreateDescriptorImageInfo(VkImageView& imageView, VkSampler& sampler, VkImageLayout layout);
	void CreateWriteDescriptorSet(VkDescriptorType type, VkDescriptorSet& DescriptorSet, VkDescriptorImageInfo* ImageInfo, VkDescriptorBufferInfo* BufferInfo, std::vector<VkWriteDescriptorSet>& descriptorWrites);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void createCommandBuffers();
	void createSyncObjects();
	void mainLoop();
	void drawFrame();
	void cleanUp();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto app = reinterpret_cast<VulkanTutorial*>(glfwGetWindowUserPointer(window));
		app->frameBufferResized = true;
	}

protected:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	// represents an abstract type of surface to present rendered images to. It's optional if you just need off-screen rendering.
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFrameBuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<Vertex> vertices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	std::vector<glm::mat4> instances;
	VkBuffer instanceBuffer;
	VkDeviceMemory instanceBufferMemory;

	std::vector<uint32_t> indices;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	UniformBuffer<Transform> objectTransformUniformBuffer;
	UniformBuffer<ColorUBO> colorUniformBuffer;
	UniformBuffer<Material> materialUniformBuffer;
	UniformBuffer<Light> lightUniformBuffer;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	const uint32_t WIDTH = 1920;
	const uint32_t HEIGHT = 1080;

	const std::string MODEL_PATH = "models/viking_room.obj";
	const std::string TEXTURE_PATH = "textures/viking_room.png";

	// Draw MAX_FRAMES_IN_FLIGHT frames simultaneously. 
	// But We may be using the frame 0 objects while frame 0 still be in flight.
	// So we need to synchronize CPU-GPU. Vulkan offers a second type of synchronization primitive called fences
	// we actually wait for them in our own code instead in vulkan functions like semaphores.
	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;
	std::vector<VkSemaphore> imageAvailableSemaphore; // before rendering
	std::vector<VkSemaphore> renderFinishedSemaphore; // after rendering
	std::vector<VkFence> inFlightFences;
	// ������ ����ȭ�� ���� �̹��� ����ȭ���� �Ѵ�. 
	// MAX_FRAMES_IN_FLIGHT�� �̹��� ������ �� ���ų�, ���� Swapchain���� �ִ� �̹��� ������ ���׹����̶�� 
	// ������������ Inflight�� �� ���� �ȴ�. �׷��� �̹��� ���� ����ȭ�� �����ν� ������ ����ȭ�� Ǯ���� ���� �̹����� ���� �� ��� 
	// ���߰� �Ѵ�. 
	std::vector<VkFence> imagesInFlight;

	bool frameBufferResized = false;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions = {
		"VK_KHR_swapchain"
	};

	Camera camera;
};

extern std::vector<char> readFile(const std::string& filename);