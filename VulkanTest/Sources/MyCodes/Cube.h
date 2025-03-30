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
            mesh.vertexBuffer.vertices = {
                {{-1.0f, -1.0f,  1.0f}, {0.0f, 0.0f}},
                {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f}},
                {{ 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},
                {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f}},

                {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
                {{-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}},
                {{ 1.0f,  1.0f, -1.0f}, {1.0f, 1.0f}},
                {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},

                {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f}},
                {{-1.0f,  1.0f,  1.0f}, {0.0f, 1.0f}},
                {{ 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},
                {{ 1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}},

                {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
                {{ 1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
                {{ 1.0f, -1.0f,  1.0f}, {1.0f, 1.0f}},
                {{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},

                {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
                {{ 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f}},
                {{ 1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},
                {{ 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f}},

                {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
                {{-1.0f, -1.0f,  1.0f}, {0.0f, 1.0f}},
                {{-1.0f,  1.0f,  1.0f}, {1.0f, 1.0f}},
                {{-1.0f,  1.0f, -1.0f}, {1.0f, 0.0f}}
            };
        }
        else if constexpr (std::is_same_v<T, VertexOnlyPos>) {
            mesh.vertexBuffer.vertices = {
                {{-1.0f, -1.0f,  1.0f}},
                {{ 1.0f, -1.0f,  1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{-1.0f,  1.0f,  1.0f}},

                {{-1.0f, -1.0f, -1.0f}},
                {{-1.0f,  1.0f, -1.0f}},
                {{ 1.0f,  1.0f, -1.0f}},
                {{ 1.0f, -1.0f, -1.0f}},

                {{-1.0f,  1.0f, -1.0f}},
                {{-1.0f,  1.0f,  1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{ 1.0f,  1.0f, -1.0f}},

                {{-1.0f, -1.0f, -1.0f}},
                {{ 1.0f, -1.0f, -1.0f}},
                {{ 1.0f, -1.0f,  1.0f}},
                {{-1.0f, -1.0f,  1.0f}},

                {{ 1.0f, -1.0f, -1.0f}},
                {{ 1.0f,  1.0f, -1.0f}},
                {{ 1.0f,  1.0f,  1.0f}},
                {{ 1.0f, -1.0f,  1.0f}},

                {{-1.0f, -1.0f, -1.0f}},
                {{-1.0f, -1.0f,  1.0f}},
                {{-1.0f,  1.0f,  1.0f}},
                {{-1.0f,  1.0f, -1.0f}}
            };
        }
        else if constexpr (std::is_same_v<T, Vertex>) {
            const glm::vec3 frontNormal = glm::vec3(0.0f, 0.0f, 1.0f);
            const glm::vec3 backNormal = glm::vec3(0.0f, 0.0f, -1.0f);
            const glm::vec3 topNormal = glm::vec3(0.0f, 1.0f, 0.0f);
            const glm::vec3 bottomNormal = glm::vec3(0.0f, -1.0f, 0.0f);
            const glm::vec3 rightNormal = glm::vec3(1.0f, 0.0f, 0.0f);
            const glm::vec3 leftNormal = glm::vec3(-1.0f, 0.0f, 0.0f);

            const glm::vec3 frontTangent = glm::vec3(1.0f, 0.0f, 0.0f);
            const glm::vec3 frontBitangent = glm::vec3(0.0f, 1.0f, 0.0f);

            const glm::vec3 backTangent = glm::vec3(-1.0f, 0.0f, 0.0f);
            const glm::vec3 backBitangent = glm::vec3(0.0f, 1.0f, 0.0f);

            const glm::vec3 topTangent = glm::vec3(1.0f, 0.0f, 0.0f);
            const glm::vec3 topBitangent = glm::vec3(0.0f, 0.0f, -1.0f);

            const glm::vec3 bottomTangent = glm::vec3(1.0f, 0.0f, 0.0f);
            const glm::vec3 bottomBitangent = glm::vec3(0.0f, 0.0f, 1.0f);

            const glm::vec3 rightTangent = glm::vec3(0.0f, 0.0f, -1.0f);
            const glm::vec3 rightBitangent = glm::vec3(0.0f, 1.0f, 0.0f);

            const glm::vec3 leftTangent = glm::vec3(0.0f, 0.0f, 1.0f);
            const glm::vec3 leftBitangent = glm::vec3(0.0f, 1.0f, 0.0f);

            const glm::vec3 defaultColor = glm::vec3(1.0f, 1.0f, 1.0f);

            mesh.vertexBuffer.vertices = {
                {{-1.0f, -1.0f,  1.0f}, defaultColor, {0.0f, 0.0f}, frontNormal, frontTangent, frontBitangent},
                {{ 1.0f, -1.0f,  1.0f}, defaultColor, {1.0f, 0.0f}, frontNormal, frontTangent, frontBitangent},
                {{ 1.0f,  1.0f,  1.0f}, defaultColor, {1.0f, 1.0f}, frontNormal, frontTangent, frontBitangent},
                {{-1.0f,  1.0f,  1.0f}, defaultColor, {0.0f, 1.0f}, frontNormal, frontTangent, frontBitangent},

                {{-1.0f, -1.0f, -1.0f}, defaultColor, {0.0f, 0.0f}, backNormal, backTangent, backBitangent},
                {{-1.0f,  1.0f, -1.0f}, defaultColor, {0.0f, 1.0f}, backNormal, backTangent, backBitangent},
                {{ 1.0f,  1.0f, -1.0f}, defaultColor, {1.0f, 1.0f}, backNormal, backTangent, backBitangent},
                {{ 1.0f, -1.0f, -1.0f}, defaultColor, {1.0f, 0.0f}, backNormal, backTangent, backBitangent},

                {{-1.0f,  1.0f, -1.0f}, defaultColor, {0.0f, 0.0f}, topNormal, topTangent, topBitangent},
                {{-1.0f,  1.0f,  1.0f}, defaultColor, {0.0f, 1.0f}, topNormal, topTangent, topBitangent},
                {{ 1.0f,  1.0f,  1.0f}, defaultColor, {1.0f, 1.0f}, topNormal, topTangent, topBitangent},
                {{ 1.0f,  1.0f, -1.0f}, defaultColor, {1.0f, 0.0f}, topNormal, topTangent, topBitangent},

                {{-1.0f, -1.0f, -1.0f}, defaultColor, {0.0f, 0.0f}, bottomNormal, bottomTangent, bottomBitangent},
                {{ 1.0f, -1.0f, -1.0f}, defaultColor, {1.0f, 0.0f}, bottomNormal, bottomTangent, bottomBitangent},
                {{ 1.0f, -1.0f,  1.0f}, defaultColor, {1.0f, 1.0f}, bottomNormal, bottomTangent, bottomBitangent},
                {{-1.0f, -1.0f,  1.0f}, defaultColor, {0.0f, 1.0f}, bottomNormal, bottomTangent, bottomBitangent},

                {{ 1.0f, -1.0f, -1.0f}, defaultColor, {0.0f, 0.0f}, rightNormal, rightTangent, rightBitangent},
                {{ 1.0f,  1.0f, -1.0f}, defaultColor, {0.0f, 1.0f}, rightNormal, rightTangent, rightBitangent},
                {{ 1.0f,  1.0f,  1.0f}, defaultColor, {1.0f, 1.0f}, rightNormal, rightTangent, rightBitangent},
                {{ 1.0f, -1.0f,  1.0f}, defaultColor, {1.0f, 0.0f}, rightNormal, rightTangent, rightBitangent},

                {{-1.0f, -1.0f, -1.0f}, defaultColor, {0.0f, 0.0f}, leftNormal, leftTangent, leftBitangent},
                {{-1.0f, -1.0f,  1.0f}, defaultColor, {0.0f, 1.0f}, leftNormal, leftTangent, leftBitangent},
                {{-1.0f,  1.0f,  1.0f}, defaultColor, {1.0f, 1.0f}, leftNormal, leftTangent, leftBitangent},
                {{-1.0f,  1.0f, -1.0f}, defaultColor, {1.0f, 0.0f}, leftNormal, leftTangent, leftBitangent}
            };
        }
        else
        {
            assert(false);
        }

        mesh.indexBuffer.indices = { 
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4, 
            8, 9, 10, 10, 11, 8, 
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
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