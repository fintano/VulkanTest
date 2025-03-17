#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <vector>
#include "VulkanTools.h"

namespace vk
{
	namespace desc
	{
		void createDescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType Type, VkShaderStageFlags Stage, std::vector<VkDescriptorSetLayoutBinding>& bindings)
		{
			VkDescriptorSetLayoutBinding Layoutbinding{};
			Layoutbinding.binding = binding;
			Layoutbinding.descriptorType = Type;
			Layoutbinding.descriptorCount = 1;
			Layoutbinding.stageFlags = Stage;
			Layoutbinding.pImmutableSamplers = nullptr;

			bindings.push_back(Layoutbinding);
		}

		void createDescriptorSetLayout(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>& bindings, VkDescriptorSetLayout& outDescriptorSetLayout)
		{
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &outDescriptorSetLayout));
		}
	}
}