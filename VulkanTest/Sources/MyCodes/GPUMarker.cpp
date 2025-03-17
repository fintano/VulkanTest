#include "GPUMarker.h"

PFN_vkCmdBeginDebugUtilsLabelEXT GPUMarker::vkCmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT GPUMarker::vkCmdEndDebugUtilsLabelEXT = nullptr;
PFN_vkCmdInsertDebugUtilsLabelEXT GPUMarker::vkCmdInsertDebugUtilsLabelEXT = nullptr;

void GPUMarker::setup(VkInstance instance)
{
	// Vulkan은 Extensions를 이렇게 Dynamically 로딩을 해야한다. 
	vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
	vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
	vkCmdInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT"));
}

GPUMarker::GPUMarker(VkCommandBuffer inCommandBuffer, const char* label)
{
	commandBuffer = inCommandBuffer;

	VkDebugUtilsLabelEXT debugLabel{};
	debugLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	debugLabel.pLabelName = label;
	debugLabel.color[0] = 1.0f;
	debugLabel.color[1] = 1.0f;
	debugLabel.color[2] = 1.0f;
	debugLabel.color[3] = 1.0f;

	vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &debugLabel);
}

GPUMarker::~GPUMarker()
{
	vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}