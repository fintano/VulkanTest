#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <vector>
#include "VulkanTools.h"

namespace vk
{
	namespace desc
	{
		void createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType Type, VkShaderStageFlags Stage, std::vector<VkDescriptorSetLayoutBinding>& bindings);
		VkDescriptorSetLayout createDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& bindings);
	}
}