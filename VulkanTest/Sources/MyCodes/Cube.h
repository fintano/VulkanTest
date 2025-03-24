#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include "Buffer.h"
#include "Vertex.h"

class VulkanTutorial;

struct Cube
{
	void createMesh(VulkanTutorial* engine);
	void cleanUp(VkDevice device);

	GPUMeshBuffers<VertexOnlyPos> mesh;
};