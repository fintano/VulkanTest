#pragma once

#include "vk_types.h"

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
#include "UniformBufferTypes.h"
#include "Vertex.h"
#include <mutex>

#include "vk_engine.h"
#include "Vk_loader.h"

#ifndef DEBUG_MODEL 
#define DEBUG_MODEL 0
#endif

typedef unsigned char stbi_uc;

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

class VulkanTutorial {
public:

	VulkanTutorial();

	void run();

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	struct GeometryPass
	{
		std::shared_ptr<AllocatedImage> position, normal, albedo, arm /* ao, roughness, metallic */ , emissive;
		VkRenderPass renderPass;
		VkFramebuffer frameBuffer;
	} geometry;

	struct ForwardPass
	{
		VkRenderPass renderPass;
		std::vector<VkFramebuffer> frameBuffers;
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
	} forward;

	VkDescriptorPool descriptorPool;

	template<typename T>
	void createVertexBuffer(const std::vector<T>& vertices, VkBuffer& outVertexBuffer, VkDeviceMemory& outVertexBufferMemory)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(*device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outVertexBuffer, outVertexBufferMemory);
		copyBuffer(stagingBuffer, outVertexBuffer, bufferSize);

		vkDestroyBuffer(*device, stagingBuffer, nullptr);
		vkFreeMemory(*device, stagingBufferMemory, nullptr);
	}
	void createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& outIndexBuffer, VkDeviceMemory& outIndexBufferMemory);
	std::shared_ptr<AllocatedImage> createTexture2D(const char* filePath, VkFormat inFormat, VkImageUsageFlagBits inUsageFlag = VK_IMAGE_USAGE_SAMPLED_BIT, const char* name = "texture");
	std::shared_ptr<AllocatedImage> createTexture2D(const std::string& filePath, VkFormat inFormat, VkImageUsageFlagBits inUsageFlag = VK_IMAGE_USAGE_SAMPLED_BIT, const char* name = "texture");
	std::shared_ptr<AllocatedImage> createTexture2D(stbi_uc* data, VkExtent3D imageSize, VkFormat format, VkImageUsageFlagBits usageFlag = VK_IMAGE_USAGE_SAMPLED_BIT, const char* name = "texture", int channelNum = 4);
	std::shared_ptr<AllocatedImage> createTexture2Df(float* inData, VkExtent3D inImageSize, VkFormat inFormat, VkImageUsageFlagBits inUsageFlag, const char* name, int channelNum);
	std::shared_ptr<AllocatedImage> createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const char* name = "none", uint32_t arrayLayers = 1, VkImageViewCreateFlags flags = 0);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t baseArrayLayer = 0, uint32_t baseMipLevel = 0);
	VkImageView createImageViewCube(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t baseArrayLayer = 0);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void generateMipmaps(VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount);
	void transitionImageLayout(VkCommandBuffer CommandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount = 1, uint32_t baseMipLevel = 0);
	void markCommandBufferRecreation();

	VkDevice getDevice() const { return *device; }
	DevicePtr getDevicePtr() const { return device; }

protected:
	virtual VkDescriptorSetLayout getGlobalDescriptorSetLayout() { return nullptr; }
	virtual void initWindow();
	virtual void initVulkan();
	virtual void processInput();
	virtual void createUniformBuffers();
	virtual void createDescriptorPool();
	virtual void createDescriptorSets();
	virtual void onPostInitVulkan();
	virtual void createRenderPass();
	virtual void updateUniformBuffer(uint32_t currentImage);
	virtual void clearUniformBuffer(uint32_t i);
	virtual void createDescriptorSetLayouts();
	virtual void createGraphicsPipelines();
	virtual void recordCommandBuffer(VkCommandBuffer commandbuffer, size_t index); 
	virtual void recordRenderPassCommands(VkCommandBuffer commandBuffer, size_t index);
	virtual void recordLightingRenderPassCommands(VkCommandBuffer commandBuffer, size_t index);
	virtual void cleanUpSwapchain();
	virtual void loadModels();
	virtual void createBuffers();
	virtual void recreateSwapChain();
	virtual void preDrawFrame(uint32_t imageIndex);
	virtual void drawFrame();
	virtual void postDrawFrame(uint32_t imageIndex);
	virtual void createCommandPool();
	virtual void createCommandBuffers();
	virtual void createFrameBuffers();
	virtual void cleanUp();
	virtual void recordForwardPassCommands(VkCommandBuffer commandBuffer, size_t index);

protected:
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
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createImageViews();
	void createTextureImageView();
	void createTextureSampler();
	void createDescriptorSetLayoutBinding(VkDescriptorType Type, VkShaderStageFlags Stage, std::vector<VkDescriptorSetLayoutBinding>& bindings);
	void createPipelineLayout(const VkDescriptorSetLayout& inDescriptorSetLayout, VkPipelineLayout& outPipelineLayout);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createColorResources();
	void createDepthResources();
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
	void createTextureImage();
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void loadModel(const std::string& modelPath, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);
	void createCommandBuffer(int32_t i);
	void tryRecreateCommandBuffer(int32_t imageIndex);
	VkDescriptorBufferInfo createDescriptorBufferInfo(VkBuffer& buffer, VkDeviceSize bufferSize);
	VkDescriptorImageInfo CreateDescriptorImageInfo(VkImageView& imageView, VkSampler& sampler, VkImageLayout layout);
	void CreateWriteDescriptorSet(VkDescriptorType type, VkDescriptorSet& DescriptorSet, VkDescriptorImageInfo* ImageInfo, VkDescriptorBufferInfo* BufferInfo, std::vector<VkWriteDescriptorSet>& descriptorWrites);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);
	void createSyncObjects();
	void mainLoop();

	void addCommandBuffer(VkCommandBuffer commandBuffer);
	size_t getCommandBufferCount();
	const VkCommandBuffer* getCommandBufferData();
	void clearCommandBuffers();
	void createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout* outDescriptorSetLayout);

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
	std::vector<VkFramebuffer> swapChainFrameBuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> commandBuffersToSubmit;

	std::vector<VkDescriptorSet> descriptorSets;

	uint32_t mipLevels;
	std::shared_ptr<AllocatedImage> defaultTexture;
	VkSampler textureSampler;

	std::shared_ptr<AllocatedImage> whiteTexture;

	// 라이팅 패스
	struct LightingPass
	{
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;
		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkDescriptorSet> descriptorSets;
	} lightingPass;
	
	std::shared_ptr<AllocatedImage> depth;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	const uint32_t WIDTH = 1920;
	const uint32_t HEIGHT = 1080;

	const std::string TEXTURE_PATH = "textures/viking_room.png";
	const std::string WHITE_TEXTURE_PATH = "textures/white_texture.png";

	// Draw MAX_FRAMES_IN_FLIGHT frames simultaneously. 
	// But We may be using the frame 0 objects while frame 0 still be in flight.
	// So we need to synchronize CPU-GPU. Vulkan offers a second type of synchronization primitive called fences
	// we actually wait for them in our own code instead in vulkan functions like semaphores.
	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;
	size_t totalFrame = 0;
	size_t targetFrameForCmdBufRecreation = 0;
	std::vector<VkSemaphore> imageAvailableSemaphore; // before rendering
	std::vector<VkSemaphore> renderFinishedSemaphore; // after rendering
	std::vector<VkFence> inFlightFences;
	// 프레임 동기화에 더해 이미지 동기화까지 한다. 
	// MAX_FRAMES_IN_FLIGHT가 이미지 수보다 더 많거나, 만약 Swapchain에서 주는 이미지 순서가 뒤죽박죽이라면 
	// 파이프라인이 Inflight일 때 쓰게 된다. 그래서 이미지 또한 동기화를 함으로써 프레임 동기화가 풀려도 같은 이미지를 쓰게 될 경우 
	// 멈추게 한다. 
	std::vector<VkFence> imagesInFlight;

	bool frameBufferResized = false;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions = {
		"VK_KHR_swapchain"
	};

	std::string ProgramName;
	DevicePtr device;
};

extern std::vector<char> readFile(const std::string& filename);