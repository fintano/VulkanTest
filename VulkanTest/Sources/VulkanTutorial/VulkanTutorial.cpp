#include "VulkanTutorial.h"
#include "GPUMarker.h"
#include "Vk_loader.h"

#ifndef USE_MSAA 
#define USE_MSAA 0
#endif

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "vk_loader.h"
#include "vk_resource_utils.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height);

std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		std::string errorMsg = "failed to open file ";
		errorMsg.append(filename);

		throw std::runtime_error(errorMsg);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VulkanTutorial::VulkanTutorial()
{
}

	void VulkanTutorial::run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void VulkanTutorial::initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void VulkanTutorial::initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		setupSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createImageViews();
		createRenderPass();
		createDescriptorSetLayouts();
		createGraphicsPipelines();
		createCommandPool();
		createColorResources();
		createDepthResources();
		createFrameBuffers();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		loadModels();
		createBuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		onPostInitVulkan();
		createCommandBuffers();
		createSyncObjects();
	}

	void VulkanTutorial::processInput()
	{
	}

	bool VulkanTutorial::checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void VulkanTutorial::createInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle!";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// extensions
		auto glfwExtensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}

		GPUMarker::setup(instance);
	}

	std::vector<const char*> VulkanTutorial::getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void VulkanTutorial::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void VulkanTutorial::setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void VulkanTutorial::setupSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void VulkanTutorial::pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
#if USE_MSAA
				msaaSamples = getMaxUsableSampleCount();
#else	
				msaaSamples = VK_SAMPLE_COUNT_1_BIT;
#endif
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	VkSampleCountFlagBits VulkanTutorial::getMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts =
			physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

		if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
		if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
		if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
		if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
		if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
		if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
		return VK_SAMPLE_COUNT_1_BIT;
	}

	bool VulkanTutorial::isDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		return indices.isComplete() && extensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	QueueFamilyIndices VulkanTutorial::findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;

				if (indices.isComplete())
				{
					break;
				}
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			i++;
		}

		return indices;
	}

	bool VulkanTutorial::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;

		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails VulkanTutorial::querySwapChainSupport(VkPhysicalDevice  device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		if (formatCount)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		if (presentModeCount)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &formatCount, details.presentModes.data());
		}

		return details;
	}

	void VulkanTutorial::createLogicalDevice()
	{
		QueueFamilyIndices Indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { Indices.graphicsFamily.value(), Indices.presentFamily.value() };
		float queuePriority = 1.f;

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		createInfo.enabledExtensionCount = static_cast<int32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, Indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, Indices.presentFamily.value(), 0, &presentQueue);
	}

	void VulkanTutorial::createSwapchain()
	{
		SwapChainSupportDetails swapChainSupportDetails = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupportDetails.formats);
		VkPresentModeKHR surfacePresentMode = chooseSwapPresentMode(swapChainSupportDetails.presentModes);
		VkExtent2D surfaceExtent = chooseSwapExtent(swapChainSupportDetails.capabilities);

		uint32_t imageCount = swapChainSupportDetails.capabilities.minImageCount + 1;
		if (swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapChainSupportDetails.capabilities.maxImageCount)
		{
			imageCount = swapChainSupportDetails.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = surfaceExtent;
		// unless using VR
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// we will be drawing on the images in the swapchain from the graphics queue and then submitting them on the presentation queue.
		QueueFamilyIndices Indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { Indices.graphicsFamily.value(), Indices.presentFamily.value() };

		if (Indices.graphicsFamily != Indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupportDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // ignore alpha.
		createInfo.presentMode = surfacePresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainExtent = surfaceExtent;
		swapChainImageFormat = surfaceFormat.format;
	}

	void VulkanTutorial::recreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);

		cleanUpSwapchain();

		createSwapchain();
		createImageViews();
		createRenderPass();
		createGraphicsPipelines();
		createColorResources();
		createDepthResources();
		createFrameBuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
	}

	void VulkanTutorial::preDrawFrame(uint32_t imageIndex)
	{
		updateUniformBuffer(imageIndex);
		clearCommandBuffers();
		addCommandBuffer(commandBuffers[imageIndex]);
	}

	void VulkanTutorial::postDrawFrame(uint32_t imageIndex)
	{
	}

	VkSurfaceFormatKHR VulkanTutorial::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
				availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanTutorial::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanTutorial::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	void VulkanTutorial::createImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}

	void VulkanTutorial::createTextureImageView()
	{
		defaultTexture.imageView = createImageView(defaultTexture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	}

	VkImageView VulkanTutorial::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t baseArrayLayer, uint32_t baseMipLevel)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = baseMipLevel;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image views!");
		}

		return imageView;
	}

	VkImageView VulkanTutorial::createImageViewCube(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t baseArrayLayer)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		createInfo.format = format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
		createInfo.subresourceRange.layerCount = 6;

		VkImageView imageView;
		if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image views!");
		}

		return imageView;
	}

	void VulkanTutorial::createTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // we will look at this on shader map chapter.
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.f;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = static_cast<float>(mipLevels);

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	void VulkanTutorial::createRenderPass() {
		/*
		 * Create lighting pass renderpass
		 */
		
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		/*subpass.pDepthStencilAttachment = &depthAttachmentRef;*/
#if USE_MSAA
		subpass.pResolveAttachments = &colorAttachmentResolveRef;
#endif
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

#if USE_MSAA
		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
#else
		std::array<VkAttachmentDescription, 1/*2*/> attachments = { colorAttachment/*, depthAttachment*/ };
#endif
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}

		/*
		 * Create a render pass for forward rendering
		 */

		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::array<VkAttachmentDescription, 2> forwardAttachments = { colorAttachment, depthAttachment};

		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		renderPassInfo.attachmentCount = static_cast<uint32_t>(forwardAttachments.size());
		renderPassInfo.pAttachments = forwardAttachments.data();
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &forward.renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create forward render pass!");
		}

		/*
		 * Create a render pass for deferred rendering (Basepass)
		 */ 

		// pos, normal, albedo, arm
		std::array<VkAttachmentDescription, 4> colorAttachments = {};
		for (VkAttachmentDescription& colorAttachment : colorAttachments)
		{
			colorAttachment.samples = msaaSamples;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		colorAttachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
		colorAttachments[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
		colorAttachments[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		colorAttachments[3].format = VK_FORMAT_R8G8B8A8_UNORM;

		std::array<VkAttachmentReference, 4> colorAttachmentRefs = { {
			{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
			{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
			{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
			{3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
		} };

		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		depthAttachmentRef.attachment = colorAttachments.size();

		subpass.colorAttachmentCount = colorAttachments.size();
		subpass.pColorAttachments = colorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = nullptr;

		std::array<VkAttachmentDescription, 5> DeferredAttachments =
		{ colorAttachments[0], colorAttachments[1], colorAttachments[2], colorAttachments[3], depthAttachment };

		renderPassInfo.attachmentCount = static_cast<uint32_t>(DeferredAttachments.size());
		renderPassInfo.pAttachments = DeferredAttachments.data();
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &geometry.renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create geometry render pass!");
		}
	}

	void VulkanTutorial::createDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout* outDescriptorSetLayout)
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, outDescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout.");
		}
	}

	void VulkanTutorial::createGraphicsPipelines()
	{
	}

	void VulkanTutorial::createDescriptorSetLayoutBinding(VkDescriptorType Type, VkShaderStageFlags Stage, std::vector<VkDescriptorSetLayoutBinding>& bindings)
	{
		VkDescriptorSetLayoutBinding Layoutbinding{};
		Layoutbinding.binding = static_cast<uint32_t>(bindings.size());
		Layoutbinding.descriptorType = Type;
		Layoutbinding.descriptorCount = 1;
		Layoutbinding.stageFlags = Stage;
		Layoutbinding.pImmutableSamplers = nullptr;

		bindings.push_back(Layoutbinding);
	}

	void VulkanTutorial::createPipelineLayout(const VkDescriptorSetLayout& inDescriptorSetLayout, VkPipelineLayout& outPipelineLayout)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &inDescriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &outPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	VkShaderModule VulkanTutorial::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	void VulkanTutorial::createFrameBuffers()
	{
		/**
		* For LightPass
		*/
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.layers = 1;

		swapChainFrameBuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
#if USE_MSAA
			std::array<VkImageView, 3> attachments = { colorImageView, depthImageView, swapChainImageViews[i] };
#else
			std::array<VkImageView, 1> attachments = { swapChainImageViews[i] };
#endif

			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS)
			{
				std::runtime_error("failed to create framebuffer!");
			}
		}

		/**
		* For GeometryPass
		*/

		std::array<VkImageView, 5> attachments = 
		{ geometry.position.imageView, geometry.normal.imageView, geometry.albedo.imageView, geometry.arm.imageView, depthImageView };

		framebufferInfo.renderPass = geometry.renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &geometry.frameBuffer) != VK_SUCCESS)
		{
			std::runtime_error("failed to create framebuffer!");
		}

		/**
		* For ForwardPass
		*/

		forward.frameBuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 2> forwardAttachments = { swapChainImageViews[i],depthImageView };

			framebufferInfo.renderPass = forward.renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(forwardAttachments.size());
			framebufferInfo.pAttachments = forwardAttachments.data();

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &forward.frameBuffers[i]) != VK_SUCCESS)
			{
				std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void VulkanTutorial::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool");
		}
	}

	void VulkanTutorial::createColorResources()
	{
		VkFormat colorFormat = swapChainImageFormat;

		// 먼저 렌더 타겟으로 사용되고 (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
		// 나중에 쉐이더에서 샘플링됩니다(VK_IMAGE_USAGE_SAMPLED_BIT)
		// VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT는 특별한 용도로, 이미지 데이터가 렌더 패스 내에서만 일시적으로 필요하고 나중에 접근할 필요가 없을 때 사용됩니다. 
		geometry.position = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Gbuffer_Position");
		geometry.position.imageView = createImageView(geometry.position.image, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		geometry.normal = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Gbuffer_Normal");
		geometry.normal.imageView = createImageView(geometry.normal.image, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		geometry.albedo = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Gbuffer_Albedo");
		geometry.albedo.imageView = createImageView(geometry.albedo.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		geometry.arm = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Gbuffer_ARM");
		geometry.arm.imageView = createImageView(geometry.arm.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void VulkanTutorial::createDepthResources()
	{
		VkFormat depthFormat = findDepthFormat();

		AllocatedImage allocatedDepthImage = createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "DepthStencil");
		depthImage = allocatedDepthImage.image;
		depthImageMemory = allocatedDepthImage.imageMemory;
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	}

	VkFormat VulkanTutorial::findDepthFormat()
	{
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkFormat VulkanTutorial::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	bool VulkanTutorial::hasStencilComponent(VkFormat format)
	{
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	void VulkanTutorial::createTextureImage()
	{
		// 버퍼 채우는 것과 같다. 
		// 데이터 -> (스테이징데이터) -> 버퍼   -> 바인딩.
		// 이미지 -> 스테이징버퍼     -> 이미지 -> 바인딩.
		// 이미지를 로드해서 스테이징 버퍼에 채워넣는다. 
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = Utils::loadImage(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, Utils::STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight))));

		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, (size_t)imageSize);
		vkUnmapMemory(device, stagingBufferMemory);

		Utils::freeImage(pixels);

		// VK_IMAGE_LAYOUT에 따라서 이미지 Access mask와 Pipeline Stage를 결정한다. 
		defaultTexture = createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT /* for blit */ | VK_IMAGE_USAGE_TRANSFER_DST_BIT /* for staging buffer*/ | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "DefaultTexture");
		transitionImageLayout(defaultTexture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		copyBufferToImage(stagingBuffer, defaultTexture.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		auto commandBuffer = beginSingleTimeCommands();
		generateMipmaps(commandBuffer, defaultTexture.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels, 1);
		endSingleTimeCommands(commandBuffer);

		// default white texture

		pixels = Utils::loadImage(WHITE_TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, Utils::STBI_rgb_alpha);
		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		whiteTexture = createTexture2D(pixels, { (uint32_t)texWidth ,(uint32_t)texHeight, 1 }, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_SAMPLED_BIT, "White");
	}

	AllocatedImage VulkanTutorial::createTexture2D(const char* filePath, VkFormat inFormat, VkImageUsageFlagBits inUsageFlag, const char* name)
	{
		int width, height, nrChannels;
		stbi_uc* data = Utils::loadImage(filePath, &width, &height, &nrChannels, Utils::STBI_rgb_alpha);
		return createTexture2D(data, VkExtent3D{ (uint32_t)width, (uint32_t)height, 1 }, inFormat, VK_IMAGE_USAGE_SAMPLED_BIT, filePath, 4);
	}

	AllocatedImage VulkanTutorial::createTexture2D(const std::string& filePath, VkFormat inFormat, VkImageUsageFlagBits inUsageFlag, const char* name)
	{
		int width, height, nrChannels;
		stbi_uc* data = Utils::loadImage(filePath.c_str(), &width, &height, &nrChannels, Utils::STBI_rgb_alpha);
		return createTexture2D(data, VkExtent3D{ (uint32_t)width, (uint32_t)height, 1 }, inFormat, VK_IMAGE_USAGE_SAMPLED_BIT, filePath.c_str(), 4);
	}

	AllocatedImage VulkanTutorial::createTexture2D(stbi_uc* inData, VkExtent3D inImageSize, VkFormat inFormat, VkImageUsageFlagBits inUsageFlag, const char* name, int channelNum)
	{
		assert(inData);
		// Sampler의 maxLOD는 텍스쳐의 mipLevels와 관련있다. 

		const int bytesPerChennel = inFormat == VK_FORMAT_R32G32B32A32_SFLOAT ? sizeof(float) : 1;
		const int bytesPerPixel = bytesPerChennel * channelNum;
		const int texWidth = inImageSize.width;
		const int texHeight = inImageSize.height;
		const int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight))));
		const VkDeviceSize imageSize = texWidth * texHeight * bytesPerPixel;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, inData, (size_t)imageSize);
		vkUnmapMemory(device, stagingBufferMemory);

		Utils::freeImage(inData);

		// VK_IMAGE_LAYOUT에 따라서 이미지 Access mask와 Pipeline Stage를 결정한다. 
		AllocatedImage allocatedImage = createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, inFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT /* for blit */ | VK_IMAGE_USAGE_TRANSFER_DST_BIT /* for staging buffer*/ | inUsageFlag, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, name);
		transitionImageLayout(allocatedImage.image, inFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		copyBufferToImage(stagingBuffer, allocatedImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);

		auto commandBuffer = beginSingleTimeCommands();
		generateMipmaps(commandBuffer, allocatedImage.image, inFormat, texWidth, texHeight, mipLevels, 1);
		endSingleTimeCommands(commandBuffer);

		allocatedImage.imageView = createImageView(allocatedImage.image, inFormat, VK_IMAGE_ASPECT_COLOR_BIT ,mipLevels);

		return allocatedImage;
	}

	void VulkanTutorial::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
	{
		auto commandBuffer = beginSingleTimeCommands();
		transitionImageLayout(commandBuffer, image, format, oldLayout, newLayout, mipLevels);
		endSingleTimeCommands(commandBuffer);
	}

	void VulkanTutorial::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount, uint32_t baseMipLevel)
	{
		/**
		* 이미지 레이아웃을 왜 전환하는가 ?
		* 이미지는 용도에 따라 다른 메모리 레이아웃을 사용합니다.
		* 레이아웃 전환은 GPU에서 메모리 재구성이 필요합니다.
		* 배리어는 이전 작업이 완료될 때까지 기다린 후 레이아웃을 변경하고, 그 다음 새 작업을 시작하도록 합니다.
		*/

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout; // undefine은 돈케어
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (hasStencilComponent(format))
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		barrier.subresourceRange.baseMipLevel = baseMipLevel;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;

		/**
		* srcAccessMask: 이전 레이아웃에서 이미지에 어떻게 접근했는지
		* dstAccessMask: 새 레이아웃에서 이미지에 어떻게 접근할 것인지
		* srcStage: 이전 작업이 일어나는 파이프라인 단계
		* dstStage: 새 작업이 일어날 파이프라인 단계
		*
		* VK_PIPELINE_STAGE_TRANSFER_BIT이라는 PipelineStage는 실제로 없지만 Transfer 동작이 일어나는 psuedo stage이다.
		* VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT는 돈케어다. 동작 전에 어떤 스테이지가 반드시 시작되어야하는 것은 아니기 때문에 첫 스테이지를 넣는다.
		*/
		VkPipelineStageFlagBits srcStage;
		VkPipelineStageFlagBits dstStage;
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
			(oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
		{
			barrier.srcAccessMask = 0;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else
		{
			throw std::runtime_error("unsupported layout transition!");
		}

		{
			vkCmdPipelineBarrier(commandBuffer,
				srcStage, dstStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		}
	}

	void VulkanTutorial::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0,0,0 };
		region.imageExtent = { width, height, 1 };

		{
			// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL는 현재 이미지가 사용중인 레이아웃. 이렇게 미리 설정 돼있어야한다. 
			vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}

		endSingleTimeCommands(commandBuffer);
	}

	void VulkanTutorial::generateMipmaps(VkCommandBuffer commandBuffer, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		// 그러니까 궁금한게.. 
		// 커맨드들이 커맨드 버퍼안에 차곡차곡 쌓여서 서브밋되고 실행이 되는데.. 이 커맨드 큐잉 순서가 보장이 안되잖아. 병렬적으로 실행되지. 
		// 그런데 만약 마지막 블릿 커맨드가 실행이 됐다치면 그 모든 커맨드들이 Transfer 스테이지 끝내기를 기다리고 있는다. 
		// 그런데 그 전, 그 전전 커맨드 들도 마찬가지로 다른 커맨드들이 Transfer 스테이지 끝내기를 기다리고 있는다. 
		// 그럼 그 첫번째 커맨드 또한 실행이 안돼서 데드락에 걸리는게 아닌가?

		// 배리어를 써야겠다는 생각의 발단은
		// n번째 이미지가 써져야지 n-1번째 이미지로 Blit을 할 수 있다는 것부터이다. 
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;


			/**
			*  Cmd0    Cmd1    Cmd2    Cmd3    Cmd4    Cmd5    ... 
			*	|		|		|		|		|		★		|		|
			*	|		|		|		|		★		|		|		|
			*	|		|		|		★		|		|		|		|
			*	★		★		★		|		|		|		|		| Transfer Stage
			*	|		|		|		|		|		|		|		|
			*	|		|		|		|		|		|		|		| Fragment Shader Stage	
			*	|		|		|		|		|		|		|		|
			* 
			*	커맨드 버퍼 안 커맨드는 순차적으로 실행되나 파이프라인 스테이지에선 병렬적으로 실행될 수 있다. 
			* 
			*/

			// 현재 파이프라인을 VK_PIPELINE_STAGE_TRANSFER_BIT 단계 전에서 막아 놓고 현재 GPU에서 실행 중인 모든 커맨드 들이 VK_PIPELINE_STAGE_TRANSFER_BIT를 끝낼 때
			// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL를 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL로, VK_ACCESS_TRANSFER_WRITE_BIT를 VK_ACCESS_TRANSFER_READ_BIT로 바꿔준다. 

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			for (uint32_t layer = 0; layer < layerCount; layer++)
			{
				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0,0,0 };
				blit.srcOffsets[1] = { mipWidth, mipHeight,1 };
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = layer;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = { 0,0,0 };
				blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = layer;
				blit.dstSubresource.layerCount = 1;

				// Blit은 VK_PIPELINE_STAGE_TRANSFER_BIT 오퍼레이션이다. Src 이미지는 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL로 배리어 이용해 바뀌었고, Dst 이미지는 텍스쳐 이미지 만들 때 바뀌었다. 
				vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
			}
			
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// 현재 파이프라인을 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT 단계 전에서 막아 놓고 현재 GPU에서 실행 중인 모든 커맨드 들이 VK_PIPELINE_STAGE_TRANSFER_BIT를 끝낼 때
			// VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL를 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL로, VK_ACCESS_TRANSFER_READ_BIT를 VK_ACCESS_SHADER_READ_BIT로 바꿔준다. 

			// 다른 모든 샘플링 오퍼레이션은 이 Transition이 끝나기를 기다린다. 모든 커맨드들이 아니라 VkImageMemoryBarrier라면 이미지 리소스에서만 락이 걸리지 않을까 추측한다. 
			// 그 중에서 barrier.subresourceRange.baseMipLevel 여기에 락이 걸리는듯.
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;

			/*
				// 이 두개의 vkCmdPipelineBarrier과 하나의 VkCmdBlitImage 커맨드는 다 각각 실행되는데,
				// 첫번째 vkCmdPipelineBarrier는 VK_PIPELINE_STAGE_TRANSFER_BIT에서 막혀있고,
				// 두번째 VkCmdBlitImage는 (i - 1 image가 VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) & (i image가 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 될 때까지 막혀있고,
				// 세번째 vkCmdPipelineBarrier는 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT에서 막혀있다.
				// 동기화 때문에 저 순서대로 진행되게끔 만들어졌다.
				// 아닌듯.

				// 커맨드와 파이프라인과의 관계는 무엇일까?
				// 같은 이미지라면 같은 파이프라인?에서 돌아갈 것 같은데 그럼VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT에서 기다린다는 것은 여기까지 왔다는 거고
				// 그럼 이 이미지는 이미 VK_PIPELINE_STAGE_TRANSFER_BIT단계를 넘었을 것이므로 이미 Blit은 진행된 거라 볼 수 있는것아닌가?

				// 만약 같은 이미지가 다른 파이프라인에서 돌아간다면 하나의 이미지에 VK_PIPELINE_STAGE_TRANSFER_BIT와 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT는 다른 파이프라인에서 진행된다는거고,
				// 그럼 한 파이프라인에 VK_PIPELINE_STAGE_TRANSFER_BIT 나 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT가 생략될 수도 있다는 건가?
				// 파이프라인이 모든 스테이지들을 지나면서 실행하는게 아닌가?


				 Barriers are synchronization commands; as such, they have a source scope and a destination scope, between which the barrier apples.
				 For vkCmdPipelineBarrier, the source scope is (usually) all commands given to the queue before the barrier call.
				 These commands may be in the current command buffer or a previously submitted CB within the same VkSubmitInfo batch, or a submitted CB in a previous vkQueueSubmit call.

				 The destination scope is (usually) all commands given to the queue after the barrier call.
				 Again, these may be commands in the same command buffer, in subsequently submitted CBs within the same batch, or in CBs submitted in subsequent calls to vkQueueSubmit.


				 // 첫번째 vkCmdPipelineBarrier 이 커맨드를 실행시키기 전까지 VK_PIPELINE_STAGE_TRANSFER_BIT에 해당하는 커맨드는 카피 커맨드 혹은 그 전 밉맵 레벨의 블릿 커맨드 밖에 없다.

				 // 두번째 vkCmdPipelineBarrier가 커맨드 버퍼에 있는 모든 파이프라인의 VK_PIPELINE_STAGE_TRANSFER_BIT이 끝날 때까지 기다리게 했으면
				 // 그 다음 밉맵 레벨의 첫번째 vkCmdPipelineBarrier는 필요 없는 것 아닌가? 이건 그냥 Transition 용인가?

				 // https://stackoverflow.com/questions/65944292/how-vulkan-pipeline-barrier-is-implemented-in-terms-of-gpu-or-its-driver
				 // 결론
				 // 모든 커맨드들은 각자 파이프라인의 스테이지를 지나는데 특정 스테이지만 지나게된다. 그리고 커맨드 버퍼안에 있는 각 커맨드의 스테이지들은 서로 겹치는게 허용되어 있다. 그래서 동기화가 필요하다.
				 // 첫번째 vkCmdPipelineBarrier를 실행할 때 그 전 모든 파이프라인에서 VK_PIPELINE_STAGE_TRANSFER_BIT가 지나가길 기다리고,
				 // vkCmdBlitImage를 실행한다.
				 // 그리고 두번째 vkCmdPipelineBarrier를 실행할 때 그 전 모든 파이프라인에서 VK_PIPELINE_STAGE_TRANSFER_BIT가 지나가길 기다린다.
				 // 그 다음 첫번째 vkCmdPipelineBarrier는 Blit 시작 전에 밉맵 이미지의 캐시를 Flush하고 Invalidate하기 위한 장치라고 보면될 것 같다.
			*/
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);
	}

	AllocatedImage VulkanTutorial::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const char* name, uint32_t arrayLayers, VkImageViewCreateFlags flags)
	{
		AllocatedImage image = {};

		// 가져온 버퍼 데이터로 이미지를 만든다. 
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 스테이징 이미지라면 나중에 소스로 쓸테니 첫번째 트랜지션때 텍셀들을 보존해야한다. 지금은 Destination.
		imageInfo.usage = usage; // VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // 나중에 쉐이더에서 써야하니 (샘플링 돼야하니) 샘플 비트 넣는다. 
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = numSamples; // 멀티 샘플링 관려
		imageInfo.flags = flags;

		if (vkCreateImage(device, &imageInfo, nullptr, &image.image) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image");
		}
		else
		{
			std::cout << "VkImage " << image.image << " " << name << std::endl;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image.image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties /*VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT*/);
		if (vkAllocateMemory(device, &allocInfo, nullptr, &image.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image.image, image.imageMemory, 0);

		return image;
	}
	
	void VulkanTutorial::loadModels()
	{
	}
	
	void VulkanTutorial::loadModel(const std::string& modelPath, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices)
	{
		tinyobj::attrib_t attrib;
		std::vector < tinyobj::shape_t> shapes;
		std::vector <tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str()))
		{
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.pos =
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord =
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.f, 1.f,1.f };

				vertex.normal =
				{
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(outVertices.size());
					outVertices.push_back(vertex);
				}

				outIndices.push_back(uniqueVertices[vertex]);
			}
		}
	}

	void VulkanTutorial::createBuffers()
	{
	}

	void VulkanTutorial::createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& outIndexBuffer, VkDeviceMemory& outIndexBufferMemory)
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outIndexBuffer, outIndexBufferMemory);
		copyBuffer(stagingBuffer, outIndexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void VulkanTutorial::createUniformBuffers()
	{
	}

	void VulkanTutorial::createDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size()) * 20;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size()) * 40;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size()) * 10;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanTutorial::createDescriptorSets()
	{
	}

	void VulkanTutorial::onPostInitVulkan()
	{

	}

	VkDescriptorBufferInfo VulkanTutorial::createDescriptorBufferInfo(VkBuffer& buffer, VkDeviceSize bufferSize)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = bufferSize;

		return bufferInfo;
	}

	VkDescriptorImageInfo VulkanTutorial::CreateDescriptorImageInfo(VkImageView& imageView, VkSampler& sampler, VkImageLayout layout)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = layout;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;

		return imageInfo;
	}

	void VulkanTutorial::CreateWriteDescriptorSet(VkDescriptorType type, VkDescriptorSet& DescriptorSet, VkDescriptorImageInfo* ImageInfo, VkDescriptorBufferInfo* BufferInfo, std::vector<VkWriteDescriptorSet>& descriptorWrites)
	{
		VkWriteDescriptorSet DescriptorWrite;
		DescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrite.pNext = nullptr;
		DescriptorWrite.dstSet = DescriptorSet;
		DescriptorWrite.dstBinding = static_cast<uint32_t>(descriptorWrites.size());
		DescriptorWrite.dstArrayElement = 0;
		DescriptorWrite.descriptorType = type;
		DescriptorWrite.descriptorCount = 1; // how many descriptor you update
		DescriptorWrite.pBufferInfo = BufferInfo;
		DescriptorWrite.pImageInfo = ImageInfo;

		descriptorWrites.emplace_back(std::move(DescriptorWrite));
	}

	void VulkanTutorial::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer!");
		}
		else
		{
			std::cout << "VkBuffer " << buffer << " " << std::endl;
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	uint32_t VulkanTutorial::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void VulkanTutorial::copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size)
	{
		auto commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	VkCommandBuffer VulkanTutorial::beginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // ?
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;
		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanTutorial::endSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	void VulkanTutorial::createCommandBuffers()
	{
		commandBuffers.resize(swapChainFrameBuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers.");
		}

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			createCommandBuffer(i);
		}
	}

	void VulkanTutorial::createCommandBuffer(int32_t i)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		recordCommandBuffer(commandBuffers[i], i);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void VulkanTutorial::recordCommandBuffer(VkCommandBuffer commandBuffer, size_t i)
	{
		/**
		* GeometryPass
		*/

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = geometry.renderPass;
		renderPassInfo.framebuffer = geometry.frameBuffer;
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		// define the clear values for vk_attachment_load_op_clear.
		std::array<VkClearValue, 5> clearValues{};
		clearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
		clearValues[1].color = { { 0.f, 0.f, 0.f, 0.f } };
		clearValues[2].color = { { 0.f, 0.f, 0.f, 0.f } };
		clearValues[3].color = { { 0.f, 0.f, 0.f, 0.f } };
		clearValues[4].depthStencil = { 1.f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		{
			GPUMarker Marker(commandBuffer, "Geometry Pass");
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			recordRenderPassCommands(commandBuffer, i);
			vkCmdEndRenderPass(commandBuffer);
		}

		transitionImageLayout(commandBuffer, geometry.position.image, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
		transitionImageLayout(commandBuffer, geometry.normal.image, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
		transitionImageLayout(commandBuffer, geometry.albedo.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
		transitionImageLayout(commandBuffer, geometry.arm.image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

		/**
		* LightingPass
		*/

		{
			GPUMarker Marker(commandBuffer, "Lighting Pass");
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFrameBuffers[i];

			// define the clear values for vk_attachment_load_op_clear.
			std::array<VkClearValue, 1> lightingPassClearValues{};
			lightingPassClearValues[0].color = { { 0.f, 0.f, 0.f, 0.f } };
			//lightingPassClearValues[1].depthStencil = { 1.f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(lightingPassClearValues.size());
			renderPassInfo.pClearValues = lightingPassClearValues.data();

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			recordLightingRenderPassCommands(commandBuffer, i);
			vkCmdEndRenderPass(commandBuffer);
		}

		/**
		* ForwardPass
		*/

		transitionImageLayout(commandBuffer, swapChainImages[i], swapChainImageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

		{
			GPUMarker Marker(commandBuffer, "Forward Pass");
			renderPassInfo.renderPass = forward.renderPass;
			renderPassInfo.framebuffer = forward.frameBuffers[i];
			renderPassInfo.clearValueCount = 0;
			renderPassInfo.pClearValues = nullptr;

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			recordForwardPassCommands(commandBuffer, i);
			vkCmdEndRenderPass(commandBuffer);
		}
	}

	void VulkanTutorial::recordRenderPassCommands(VkCommandBuffer commandBuffer, size_t index) {
		
	}

	void VulkanTutorial::recordLightingRenderPassCommands(VkCommandBuffer commandBuffer, size_t index)
	{

	}

	void VulkanTutorial::recordForwardPassCommands(VkCommandBuffer commandBuffer, size_t index)
	{

	}

	void VulkanTutorial::createSyncObjects()
	{
		imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void VulkanTutorial::mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			processInput();
			drawFrame();
		}

		vkDeviceWaitIdle(device);
	}

	void VulkanTutorial::drawFrame()
	{
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		/** 1 - 2 사이 ImageAvailableSemaphore, 2-3 사이 renderFinishedSemaphore 맞나? */
		/** 1. Acquire an image from the swap chain */
		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire next image!");
		}

		preDrawFrame(imageIndex);

		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		imagesInFlight[imageIndex] = inFlightFences[currentFrame];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		// Color attachment에 쓰기 직전 스테이지까지 기다리다가 세마포어가 시그널되면 (ImageIndex를 얻으면) 렌더패쓰를 진행한다. 
		VkSemaphore waitSemaphore[]{ imageAvailableSemaphore[currentFrame] };
		// VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT specifies the stage of the pipeline after blending where the final color values are output from the pipeline. 
		VkPipelineStageFlags waitStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphore;
		submitInfo.pWaitDstStageMask = waitStage;

		/** 2. Execute the command buffer with that image as attachment in the framebuffer */
		submitInfo.commandBufferCount = getCommandBufferCount();
		submitInfo.pCommandBuffers = getCommandBufferData();
		// the semaphore to signal once the command buffer have finished execution.
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer");
		}

		/** 3. Return the image to the swap chain for presentation */
		// These are executed asynchronously.
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized)
		{
			frameBufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		postDrawFrame(imageIndex);

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanTutorial::updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	}

	void VulkanTutorial::clearUniformBuffer(uint32_t i)
	{
	}

	void VulkanTutorial::createDescriptorSetLayouts()
	{
	}

	void VulkanTutorial::cleanUp()
	{
		cleanUpSwapchain();

		whiteTexture.Destroy(device);
		vkDestroySampler(device, textureSampler, nullptr);
		defaultTexture.Destroy(device);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, lightingPass.descriptorSetLayout, nullptr);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device, renderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(device, commandPool, nullptr);
		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void VulkanTutorial::addCommandBuffer(VkCommandBuffer commandBuffer)
	{
		commandBuffersToSubmit.push_back(commandBuffer);
	}

	size_t VulkanTutorial::getCommandBufferCount()
	{
		return static_cast<size_t>(commandBuffersToSubmit.size());
	}

	const VkCommandBuffer* VulkanTutorial::getCommandBufferData()
	{
		return commandBuffersToSubmit.data();
	}

	void VulkanTutorial::clearCommandBuffers()
	{
		commandBuffersToSubmit.clear();
	}

	void VulkanTutorial::cleanUpSwapchain()
	{
		vkDestroyPipelineLayout(device, lightingPass.pipelineLayout, nullptr);
		vkDestroyPipeline(device, lightingPass.pipeline, nullptr);

		geometry.position.Destroy(device);
		geometry.normal.Destroy(device);
		geometry.albedo.Destroy(device);
		geometry.arm.Destroy(device);

		vkDestroyPipelineLayout(device, forward.pipelineLayout, nullptr);
		vkDestroyPipeline(device, forward.pipeline, nullptr);
		for (size_t i = 0; i < swapChainFrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(device, forward.frameBuffers[i], nullptr);
		}

		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);
		for (size_t i = 0; i < swapChainFrameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(device, swapChainFrameBuffers[i], nullptr);
		}
		vkDestroyFramebuffer(device, geometry.frameBuffer, nullptr);
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		//vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);
		vkDestroyRenderPass(device, geometry.renderPass, nullptr);
		vkDestroyRenderPass(device, forward.renderPass, nullptr);
		for (auto imageView : swapChainImageViews)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			clearUniformBuffer(i);
		}
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}