#include "Cube.h"
#include "VulkanTutorial.h"

void Cube::createMesh(VulkanTutorial* engine)
{
	mesh.vertexBuffer.vertices = {
		// �ո� (z = 1.0f)
	   { {-1.0f, -1.0f,  1.0f} },
	   { { 1.0f, -1.0f,  1.0f} },
	   { { 1.0f,  1.0f,  1.0f} },
	   { {-1.0f,  1.0f,  1.0f} },

	   // �޸� (z = -1.0f)
	   { {-1.0f, -1.0f, -1.0f} },
	   { {-1.0f,  1.0f, -1.0f} },
	   { { 1.0f,  1.0f, -1.0f} },
	   { { 1.0f, -1.0f, -1.0f} },

	   // ���� (y = 1.0f)
	   { {-1.0f,  1.0f, -1.0f} },
	   { {-1.0f,  1.0f,  1.0f} },
	   { { 1.0f,  1.0f,  1.0f} },
	   { { 1.0f,  1.0f, -1.0f} },

	   // �Ʒ��� (y = -1.0f)
	   { {-1.0f, -1.0f, -1.0f} },
	   { { 1.0f, -1.0f, -1.0f} },
	   { { 1.0f, -1.0f,  1.0f} },
	   { {-1.0f, -1.0f,  1.0f} },

	   // ������ �� (x = 1.0f)
	   { { 1.0f, -1.0f, -1.0f} },
	   { { 1.0f,  1.0f, -1.0f} },
	   { { 1.0f,  1.0f,  1.0f} },
	   { { 1.0f, -1.0f,  1.0f} },

	   // ���� �� (x = -1.0f)
	   { {-1.0f, -1.0f, -1.0f} },
	   { {-1.0f, -1.0f,  1.0f} },
	   { {-1.0f,  1.0f,  1.0f} },
	   { {-1.0f,  1.0f, -1.0f} }
	};

	mesh.indexBuffer.indices = {
		// �ո�
		 0, 1, 2, 2, 3, 0,
		 // �޸�
		 4, 5, 6, 6, 7, 4,
		 // ����
		 8, 9, 10, 10, 11, 8,
		 // �Ʒ���
		 12, 13, 14, 14, 15, 12,
		 // ������ ��
		 16, 17, 18, 18, 19, 16,
		 // ���� ��
		 20, 21, 22, 22, 23, 20
	};

	engine->createVertexBuffer(mesh.vertexBuffer.vertices, mesh.vertexBuffer.Buffer, mesh.vertexBuffer.BufferMemory);
	engine->createIndexBuffer(mesh.indexBuffer.indices, mesh.indexBuffer.Buffer, mesh.indexBuffer.BufferMemory);
}