#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

enum class MaterialPass : uint8_t
{
	MainColor,
	Transparent,
	Other
};

struct MaterialPipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
};

struct MaterialInstance
{
	MaterialPipeline* pipeline;
	std::vector<VkDescriptorSet> materialSet;
	MaterialPass passType;
};

//struct RenderObject
//{
//	uint32_t indexCount;
//	uint32_t firstIndex;
//	VkBuffer indexBuffer;
//
//	MaterialInstance* material;
//
//	glm::mat4 transform;
//	VkDeviceAddress vertexBufferAddress;
//};

//class IRenderable
//{
//	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) = 0;
//};
