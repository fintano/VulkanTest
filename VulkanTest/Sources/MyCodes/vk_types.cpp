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
	for (int i = 0; i < imageViews.size(); i++)
	{
		for (int j = 0; j < imageViews[i].size(); j++)
		{
			vkDestroyImageView(device, imageViews[i][j], nullptr);
		}
	}
	vkDestroyImageView(device, cubeImageView, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
}