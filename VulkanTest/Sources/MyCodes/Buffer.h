#pragma once

#include "Vertex.h"

#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>

template<typename T>
struct VertexBuffer
{
	std::vector<T> vertices;
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;

	void Destroy(VkDevice device)
	{
		vkDestroyBuffer(device, Buffer, nullptr);
		vkFreeMemory(device, BufferMemory, nullptr);
	}
};

struct IndexBuffer
{
	std::vector<uint32_t> indices;
	VkBuffer Buffer;
	VkDeviceMemory BufferMemory;

	void Destroy(VkDevice device)
	{
		vkDestroyBuffer(device, Buffer, nullptr);
		vkFreeMemory(device, BufferMemory, nullptr);
	}
};

template<typename T>
struct GPUMeshBuffers
{
	VertexBuffer<T> vertexBuffer;
	IndexBuffer indexBuffer;

	void Destroy(VkDevice device)
	{
		vertexBuffer.Destroy(device);
		indexBuffer.Destroy(device);
	}
};