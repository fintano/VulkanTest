#pragma once

#include <glm/glm.hpp>

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
