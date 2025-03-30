#pragma once
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include "Buffer.h"
#include "Vertex.h"
#include "VulkanTutorial.h"

#include "VulkanTutorialExtension.h"
#include "vk_initializers.h"
#include "VulkanTools.h"
#include "vk_resource_utils.h"
#include "vk_pathes.h"

template<typename T>
struct Sphere
{
    void createMesh(VulkanTutorial* engine, unsigned int xSegments = 64, unsigned int ySegments = 64)
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        std::vector<uint32_t> indices;

        const float PI = glm::pi<float>();

        for (unsigned int y = 0; y <= ySegments; ++y)
        {
            for (unsigned int x = 0; x <= xSegments; ++x)
            {
                float xSegment = (float)x / (float)xSegments;
                float ySegment = (float)y / (float)ySegments;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uvs.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::normalize(glm::vec3(xPos, yPos, zPos)));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < ySegments; ++y)
        {
            if (!oddRow)
            {
                for (unsigned int x = 0; x <= xSegments; ++x)
                {
                    indices.push_back(y * (xSegments + 1) + x);
                    indices.push_back((y + 1) * (xSegments + 1) + x);
                }
            }
            else
            {
                for (int x = xSegments; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (xSegments + 1) + x);
                    indices.push_back(y * (xSegments + 1) + x);
                }
            }
            oddRow = !oddRow;
        }

        // 삼각형 스트립을 삼각형 목록으로 변환 (Vulkan에서는 삼각형 목록이 더 일반적)
        std::vector<uint32_t> triangleListIndices;
        for (size_t i = 0; i < indices.size() - 2; ++i)
        {
            if ((i % 2) == 0)
            {
                triangleListIndices.push_back(indices[i]);
                triangleListIndices.push_back(indices[i + 2]);
                triangleListIndices.push_back(indices[i + 1]);
            }
            else
            {
                triangleListIndices.push_back(indices[i]);
                triangleListIndices.push_back(indices[i + 1]);
                triangleListIndices.push_back(indices[i + 2]);
            }
        }

        // 정점 데이터 생성
        if constexpr (std::is_same_v<T, VertexOnlyPos>)
        {
            mesh.vertexBuffer.vertices.resize(positions.size());
            for (size_t i = 0; i < positions.size(); ++i)
            {
                mesh.vertexBuffer.vertices[i] = { positions[i] };
            }
        }
        else if constexpr (std::is_same_v<T, VertexOnlyTex>)
        {
            mesh.vertexBuffer.vertices.resize(positions.size());
            for (size_t i = 0; i < positions.size(); ++i)
            {
                mesh.vertexBuffer.vertices[i] = { positions[i], uvs[i] };
            }
        }
        else if constexpr (std::is_same_v<T, Vertex>)
        {
            // 탄젠트와 바이탄젠트 계산을 위한 준비
            std::vector<glm::vec3> tangents(positions.size(), glm::vec3(0.0f));
            std::vector<glm::vec3> bitangents(positions.size(), glm::vec3(0.0f));

            // 삼각형별로 탄젠트와 바이탄젠트 계산
            for (size_t i = 0; i < triangleListIndices.size(); i += 3)
            {
                uint32_t idx0 = triangleListIndices[i];
                uint32_t idx1 = triangleListIndices[i + 1];
                uint32_t idx2 = triangleListIndices[i + 2];

                glm::vec3 pos0 = positions[idx0];
                glm::vec3 pos1 = positions[idx1];
                glm::vec3 pos2 = positions[idx2];

                glm::vec2 uv0 = uvs[idx0];
                glm::vec2 uv1 = uvs[idx1];
                glm::vec2 uv2 = uvs[idx2];

                glm::vec3 edge1 = pos1 - pos0;
                glm::vec3 edge2 = pos2 - pos0;
                glm::vec2 deltaUV1 = uv1 - uv0;
                glm::vec2 deltaUV2 = uv2 - uv0;

                float denominator = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                glm::vec3 tangent;
                glm::vec3 bitangent;

                if (std::abs(denominator) < 0.0000001f)
                {
                    // UV 좌표가 중복되거나 문제가 있는 경우 기본값 사용
                    tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                    bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                else
                {
                    float f = 1.0f / denominator;

                    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                }

                // 길이가 0에 가까우면 기본값 사용
                if (glm::length(tangent) > 0.0000001f)
                {
                    tangent = glm::normalize(tangent);
                }
                else
                {
                    tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                }

                if (glm::length(bitangent) > 0.0000001f)
                {
                    bitangent = glm::normalize(bitangent);
                }
                else
                {
                    bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                // 각 정점에 탄젠트와 바이탄젠트 추가
                tangents[idx0] += tangent;
                tangents[idx1] += tangent;
                tangents[idx2] += tangent;

                bitangents[idx0] += bitangent;
                bitangents[idx1] += bitangent;
                bitangents[idx2] += bitangent;
            }

            // 각 정점의 최종 탄젠트와 바이탄젠트 정규화
            for (size_t i = 0; i < tangents.size(); ++i)
            {
                if (glm::length(tangents[i]) > 0.0000001f)
                {
                    tangents[i] = glm::normalize(tangents[i]);
                }
                else
                {
                    tangents[i] = glm::vec3(1.0f, 0.0f, 0.0f);
                }

                if (glm::length(bitangents[i]) > 0.0000001f)
                {
                    bitangents[i] = glm::normalize(bitangents[i]);
                }
                else
                {
                    bitangents[i] = glm::vec3(0.0f, 1.0f, 0.0f);
                }
            }

            // 정점 데이터 생성
            const glm::vec3 defaultColor = glm::vec3(1.0f, 1.0f, 1.0f);
            mesh.vertexBuffer.vertices.resize(positions.size());
            for (size_t i = 0; i < positions.size(); ++i)
            {
                mesh.vertexBuffer.vertices[i] = {
                    positions[i],
                    defaultColor,
                    uvs[i],
                    normals[i],
                    tangents[i],
                    bitangents[i]
                };
            }
        }
        else
        {
            assert(false);
        }

        // 삼각형 감김 방향 테스트 코드 추가
        {
            std::cout << "Analyzing winding order for sphere mesh...\n";
            int ccwCount = 0;
            int cwCount = 0;

            auto isCounterClockwise = [](const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& normal) -> bool {
                // 삼각형의 두 엣지 계산
                glm::vec3 edge1 = p1 - p0;
                glm::vec3 edge2 = p2 - p0;

                // 외적을 통해 삼각형의 법선 계산
                glm::vec3 computedNormal = glm::cross(edge1, edge2);

                // 계산된 법선과 주어진 법선의 방향이 일치하는지 확인
                return glm::dot(computedNormal, normal) > 0.0f;
                };

            for (size_t i = 0; i < triangleListIndices.size(); i += 3) {
                uint32_t idx0 = triangleListIndices[i];
                uint32_t idx1 = triangleListIndices[i + 1];
                uint32_t idx2 = triangleListIndices[i + 2];

                glm::vec3 p0 = positions[idx0];
                glm::vec3 p1 = positions[idx1];
                glm::vec3 p2 = positions[idx2];
                glm::vec3 normal = normals[idx0]; // 구의 경우 정점 위치가 정규화된 법선과 동일

                bool isCCW = isCounterClockwise(p0, p1, p2, normal);

                if (isCCW) {
                    ccwCount++;
                }
                else {
                    cwCount++;
                }

                // 첫 10개 삼각형의 분석 결과 출력
                if (i / 3 < 10) {
                    std::cout << "Triangle " << (i / 3) << ": "
                        << (isCCW ? "CCW" : "CW") << "\n";
                    std::cout << "  Positions: ("
                        << p0.x << ", " << p0.y << ", " << p0.z << "), ("
                        << p1.x << ", " << p1.y << ", " << p1.z << "), ("
                        << p2.x << ", " << p2.y << ", " << p2.z << ")\n";
                    std::cout << "  Normal: ("
                        << normal.x << ", " << normal.y << ", " << normal.z << ")\n";
                }
            }

            std::cout << "Winding order analysis complete: "
                << ccwCount << " CCW triangles, "
                << cwCount << " CW triangles\n";

            if (ccwCount > 0 || cwCount > 0) {
                float consistencyPercentage = (max(ccwCount, cwCount)) / (float)(ccwCount + cwCount) * 100.0f;
                std::cout << "Sphere is primarily "
                    << (ccwCount > cwCount ? "COUNTER-CLOCKWISE" : "CLOCKWISE")
                    << " (" << consistencyPercentage << "% consistent)\n";

                if (ccwCount > cwCount) {
                    std::cout << "For Vulkan rendering with Y-flip, use VK_FRONT_FACE_CLOCKWISE\n";
                }
                else {
                    std::cout << "For Vulkan rendering with Y-flip, use VK_FRONT_FACE_COUNTER_CLOCKWISE\n";
                }
            }
            else {
                std::cout << "No valid triangles found for winding analysis!\n";
            }
        }

        mesh.indexBuffer.indices = triangleListIndices;

        engine->createVertexBuffer(mesh.vertexBuffer.vertices, mesh.vertexBuffer.Buffer, mesh.vertexBuffer.BufferMemory);
        engine->createIndexBuffer(mesh.indexBuffer.indices, mesh.indexBuffer.Buffer, mesh.indexBuffer.BufferMemory);
    }

    void cleanUp(VkDevice device)
    {
        mesh.Destroy(device);
    }

    GPUMeshBuffers<T> mesh;
};