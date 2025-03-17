#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Vertex.h"

struct Texture
{
	VkDeviceMemory              Memory;
	VkImage                     Image;
	VkImageView                 ImageView;
	VkDescriptorSet             DescriptorSet;

	Texture() { memset(this, 0, sizeof(*this)); }
};

struct IndexBuffer
{
	std::vector<uint32_t> indices;
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;
};

struct VertexBuffer
{
	std::vector<Vertex> vertices;
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;
};

struct GPUMeshBuffers
{
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
};

enum class MaterialPass :uint8_t {
	MainColor,
	Transparent,
	Other
};
struct MaterialPipeline {
	VkPipeline pipeline;
	VkPipelineLayout layout;
};

struct MaterialInstance {
	MaterialPipeline* pipeline;
	VkDescriptorSet materialSet;
	MaterialPass passType;
};

// push constants for our mesh object draws
struct GPUDrawPushConstants {
	glm::mat4 worldMatrix;
	VkBuffer vertexBuffer;
};