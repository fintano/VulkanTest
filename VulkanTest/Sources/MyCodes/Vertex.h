#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <vector>

struct Vertex
{
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(16) glm::vec2 texCoord;
	alignas(16) glm::vec3 normal;

	// Bindings: spacing between data and whether the data is per-vertex or
	// per - instance(see instancing)
	// Stride를 정하고 이게 Vertex인지 Instance인지 정하는 듯. 
	static void getBindingDescriptions(std::vector<VkVertexInputBindingDescription>& bindingDescriptions)
	{
		//VkVertexInputBindingDescription bindingDescription;
		bindingDescriptions.emplace_back(VkVertexInputBindingDescription{ 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX });

		/**
		 * All of our per-vertex data is packed together in one array, so we're only going to have one binding.
		 * The 'binding' parameter specifies the index of the binding in the array of bindings.
		 */
		 //bindingDescription.binding = 0; 
		 //bindingDescription.stride = sizeof(Vertex);
		 //bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	// Attribute descriptions: type of the attributes passed to the vertex shader,
	// which binding to load them from and at which offset
	static auto getAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		// Per-vertex attributes
		// These are advanced for each vertex fetched by the vertex shader
		attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) });
		attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{ 2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord) });
		attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord && normal == other.normal;
	}
};

namespace std {
	// Hash specialization for glm::vec2
	template<> struct hash<glm::vec2> {
		size_t operator()(const glm::vec2& v) const {
			size_t hash = 0;
			hash_combine(hash, v.x);
			hash_combine(hash, v.y);
			return hash;
		}

		// Helper function to combine hash values
		static void hash_combine(size_t& seed, float v) {
			seed ^= std::hash<float>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};

	// Hash specialization for glm::vec3
	template<> struct hash<glm::vec3> {
		size_t operator()(const glm::vec3& v) const {
			size_t hash = 0;
			hash_combine(hash, v.x);
			hash_combine(hash, v.y);
			hash_combine(hash, v.z);
			return hash;
		}

		// Helper function to combine hash values
		static void hash_combine(size_t& seed, float v) {
			seed ^= std::hash<float>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};

	// Now we can define the hash for Vertex
	template<> struct hash<Vertex> {
		size_t operator()(const Vertex& vertex) const {
			size_t seed = 0;
			hash_combine(seed, vertex.pos);
			hash_combine(seed, vertex.color);
			hash_combine(seed, vertex.texCoord);
			hash_combine(seed, vertex.normal);
			return seed;
		}

		// Helper function to combine hash values with vec3
		static void hash_combine(size_t& seed, const glm::vec3& v) {
			seed ^= std::hash<glm::vec3>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}

		// Helper function to combine hash values with vec2
		static void hash_combine(size_t& seed, const glm::vec2& v) {
			seed ^= std::hash<glm::vec2>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};
}

struct Instance 
{
	glm::mat4 model;

	// Bindings: spacing between data and whether the data is per-vertex or
	// per - instance(see instancing)
	// Stride를 정하고 이게 Vertex인지 Instance인지 정하는 듯. 
	static void getBindingDescriptions(std::vector<VkVertexInputBindingDescription>& bindingDescriptions)
	{
		bindingDescriptions.emplace_back(VkVertexInputBindingDescription{ 1, sizeof(glm::vec4) * 4, VK_VERTEX_INPUT_RATE_INSTANCE });
	}

	// Attribute descriptions: type of the attributes passed to the vertex shader,
	// which binding to load them from and at which offset
	static void getAttributeDescriptions(std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		// Per-Instance attributes
		// These are advanced for each instance rendered
		attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{ 4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0 });
		attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{ 5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 1 });
		attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{ 6, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 2 });
		attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{ 7, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 3 });

	}

	bool operator==(const Instance& other) const
	{
		return model == other.model;
	}
};
