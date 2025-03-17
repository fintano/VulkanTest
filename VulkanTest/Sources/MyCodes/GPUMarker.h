#pragma once

#include "vulkan/vulkan.h"

class GPUMarker
{
	static PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT;
	static PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
	static PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;

public:
	static void setup(VkInstance instance);

	GPUMarker(VkCommandBuffer inCommandBuffer, const char* label);
	~GPUMarker();

private:
	VkCommandBuffer commandBuffer;
};
