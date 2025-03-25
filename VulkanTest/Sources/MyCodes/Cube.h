#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include "Buffer.h"
#include "Vertex.h"

class VulkanTutorial;

template<typename T>
struct Cube
{
	void createMesh(VulkanTutorial* engine)
	{
		if constexpr (std::is_same_v<T, VertexOnlyTex>) {
			// ÅØ½ºÃ³ ÁÂÇ¥¸¸ ÀÖ´Â °æ¿ì (vec2 texCoord)
			mesh.vertexBuffer.vertices = {
				// ¾Õ¸é
				{ {-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f} },  // ÁÂÇÏ´Ü
				{ { 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f} },  // ¿ìÇÏ´Ü
				{ { 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f} },  // ¿ì»ó´Ü
				{ {-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f} },  // ÁÂ»ó´Ü
				// µÞ¸é
				{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f} },
				{ {-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f} },
				{ { 1.0f,  1.0f, -1.0f}, {1.0f, 1.0f} },
				{ { 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f} },
				// À­¸é
				{ {-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f} },
				{ {-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f} },
				{ { 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f} },
				{ { 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f} },
				// ¾Æ·§¸é
				{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f} },
				{ { 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f} },
				{ { 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f} },
				{ {-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f} },
				// ¿À¸¥ÂÊ ¸é
				{ { 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f} },
				{ { 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f} },
				{ { 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f} },
				{ { 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f} },
				// ¿ÞÂÊ ¸é
				{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f} },
				{ {-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f} },
				{ {-1.0f,  1.0f,  1.0f}, {1.0f, 1.0f} },
				{ {-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f} }
			};
		}
		else if constexpr (std::is_same_v<T, VertexOnlyPos>) {

			mesh.vertexBuffer.vertices = {
				// ¾Õ¸é (z = 1.0f)
			   { {-1.0f, -1.0f,  1.0f} },
			   { { 1.0f, -1.0f,  1.0f} },
			   { { 1.0f,  1.0f,  1.0f} },
			   { {-1.0f,  1.0f,  1.0f} },

			   // µÞ¸é (z = -1.0f)
			   { {-1.0f, -1.0f, -1.0f} },
			   { {-1.0f,  1.0f, -1.0f} },
			   { { 1.0f,  1.0f, -1.0f} },
			   { { 1.0f, -1.0f, -1.0f} },

			   // À­¸é (y = 1.0f)
			   { {-1.0f,  1.0f, -1.0f} },
			   { {-1.0f,  1.0f,  1.0f} },
			   { { 1.0f,  1.0f,  1.0f} },
			   { { 1.0f,  1.0f, -1.0f} },

			   // ¾Æ·§¸é (y = -1.0f)
			   { {-1.0f, -1.0f, -1.0f} },
			   { { 1.0f, -1.0f, -1.0f} },
			   { { 1.0f, -1.0f,  1.0f} },
			   { {-1.0f, -1.0f,  1.0f} },

			   // ¿À¸¥ÂÊ ¸é (x = 1.0f)
			   { { 1.0f, -1.0f, -1.0f} },
			   { { 1.0f,  1.0f, -1.0f} },
			   { { 1.0f,  1.0f,  1.0f} },
			   { { 1.0f, -1.0f,  1.0f} },

			   // ¿ÞÂÊ ¸é (x = -1.0f)
			   { {-1.0f, -1.0f, -1.0f} },
			   { {-1.0f, -1.0f,  1.0f} },
			   { {-1.0f,  1.0f,  1.0f} },
			   { {-1.0f,  1.0f, -1.0f} }
			};
		}
		else
		{
			assert(false);
		}

		mesh.indexBuffer.indices = {
			// ¾Õ¸é
			 0, 1, 2, 2, 3, 0,
			 // µÞ¸é
			 4, 5, 6, 6, 7, 4,
			 // À­¸é
			 8, 9, 10, 10, 11, 8,
			 // ¾Æ·§¸é
			 12, 13, 14, 14, 15, 12,
			 // ¿À¸¥ÂÊ ¸é
			 16, 17, 18, 18, 19, 16,
			 // ¿ÞÂÊ ¸é
			 20, 21, 22, 22, 23, 20
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