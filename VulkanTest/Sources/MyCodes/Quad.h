#pragma once
#include "VulkanTutorial.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include "Buffer.h"
#include "Vertex.h"

template<typename T>
class Quad
{
public:
	void createMesh(VulkanTutorial* engine)
	{
		if constexpr (std::is_same_v<T, VertexOnlyTex>) {
			// For texture-only vertices (using vec2 texCoord)
			mesh.vertexBuffer.vertices = {
				{ {-1.0f, -1.0f, 0.0f} , {0.0f, 0.0f} },  // 좌하단
				{ { 1.0f, -1.0f, 0.0f} , {1.0f, 0.0f} },  // 우하단
				{ { 1.0f,  1.0f, 0.0f} , {1.0f, 1.0f} },  // 우상단
				{ {-1.0f,  1.0f, 0.0f} , {0.0f, 1.0f} }   // 좌상단
			};
		}
		else if constexpr (std::is_same_v<T, VertexOnlyPos>) {
			// For vertices with position
			mesh.vertexBuffer.vertices = {
				{ {-1.0f, -1.0f, 0.0f} },  // 좌하단
				{ { 1.0f, -1.0f, 0.0f} },  // 우하단
				{ { 1.0f,  1.0f, 0.0f} },  // 우상단
				{ {-1.0f,  1.0f, 0.0f} }   // 좌상단
			};
		}
		else
		{
			assert(false);
		}

		mesh.indexBuffer.indices = {
			0, 1, 2,  // 첫 번째 삼각형
			2, 3, 0   // 두 번째 삼각형
		};

		engine->createVertexBuffer(mesh.vertexBuffer.vertices, mesh.vertexBuffer.Buffer, mesh.vertexBuffer.BufferMemory);
		engine->createIndexBuffer(mesh.indexBuffer.indices, mesh.indexBuffer.Buffer, mesh.indexBuffer.BufferMemory);
	}

	void cleanUp(VkDevice device)
	{
		mesh.Destroy(device);
	}

	GPUMeshBuffers<T> mesh;

};