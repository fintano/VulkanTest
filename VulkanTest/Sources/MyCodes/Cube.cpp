#include "Cube.h"
#include "VulkanTutorial.h"

void Cube::createMesh(VulkanTutorial* engine)
{
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