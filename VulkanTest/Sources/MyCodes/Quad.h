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
				{ {-1.0f, -1.0f, 0.0f} , {0.0f, 0.0f} },  // ���ϴ�
				{ { 1.0f, -1.0f, 0.0f} , {1.0f, 0.0f} },  // ���ϴ�
				{ { 1.0f,  1.0f, 0.0f} , {1.0f, 1.0f} },  // ����
				{ {-1.0f,  1.0f, 0.0f} , {0.0f, 1.0f} }   // �»��
			};
		}
		else if constexpr (std::is_same_v<T, VertexOnlyPos>) {
			// For vertices with position
			mesh.vertexBuffer.vertices = {
				{ {-1.0f, -1.0f, 0.0f} },  // ���ϴ�
				{ { 1.0f, -1.0f, 0.0f} },  // ���ϴ�
				{ { 1.0f,  1.0f, 0.0f} },  // ����
				{ {-1.0f,  1.0f, 0.0f} }   // �»��
			};
		}
		else
		{
			assert(false);
		}

		mesh.indexBuffer.indices = {
			0, 1, 2,  // ù ��° �ﰢ��
			2, 3, 0   // �� ��° �ﰢ��
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