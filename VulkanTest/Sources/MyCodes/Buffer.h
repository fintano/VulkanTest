#pragma once

#include "Vertex.h"

#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>


struct VertexBuffer
{
	std::vector<Vertex> vertices;
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;
};

struct IndexBuffer
{
	std::vector<uint32_t> indices;
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;
};

struct GPUMeshBuffers
{
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
};