#include "vk_types.h"

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void Node::refreshTransform(const glm::mat4& parentMatrix)
{
	worldTransform = parentMatrix * localTransform;
	for (auto c : children) {
		c->refreshTransform(worldTransform);
	}
}

AllocatedImage::AllocatedImage(DevicePtr inDevice)
	: image(VK_NULL_HANDLE)
	, imageView(VK_NULL_HANDLE)
	, imageMemory(VK_NULL_HANDLE)
{
	device = inDevice;
}

AllocatedImage::~AllocatedImage()
{
	Destroy();
}

void AllocatedImage::Destroy()
{
	if (image != VK_NULL_HANDLE)
	{
		vkDestroyImage(*device, image, nullptr);
	}

	if (imageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(*device, imageView, nullptr);
	}

	if (imageMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(*device, imageMemory, nullptr);
	}
}

CubeMap::CubeMap(DevicePtr inDevice)
{
	device = inDevice;
}

CubeMap::~CubeMap()
{
	Destroy();
}

void CubeMap::Destroy()
{
	for (int i = 0; i < imageViews.size(); i++)
	{
		for (int j = 0; j < imageViews[i].size(); j++)
		{
			if (imageViews[i][j] != VK_NULL_HANDLE)
			{
				vkDestroyImageView(*device, imageViews[i][j], nullptr);
			}
		}
	}
}

DeviceWrapper::~DeviceWrapper()
{
	if (device != VK_NULL_HANDLE) {
		vkDestroyDevice(device, nullptr);

		if (debugMessenger != VK_NULL_HANDLE) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}
}