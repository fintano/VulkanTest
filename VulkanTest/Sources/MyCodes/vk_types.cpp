#include "vk_types.h"

void Node::refreshTransform(const glm::mat4& parentMatrix)
{
	worldTransform = parentMatrix * localTransform;
	for (auto c : children) {
		c->refreshTransform(worldTransform);
	}
}

void AllocatedImage::Destroy(VkDevice device)
{
	vkDestroyImage(device, image, nullptr);
	vkDestroyImageView(device, imageView, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
}

void CubeMap::Destroy(VkDevice device)
{
	vkDestroyImage(device, image, nullptr);
	for (int i = 0; i < 6; i++)
	{
		vkDestroyImageView(device, imageViews[i], nullptr);
	}
	vkDestroyImageView(device, cubeImageView, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
}