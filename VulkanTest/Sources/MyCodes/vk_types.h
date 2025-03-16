#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "Vertex.h"

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