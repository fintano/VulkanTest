#pragma once

struct Transform {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct ColorUBO {
	alignas(16) glm::vec3 objectColor;
	alignas(16) glm::vec3 lightColor;
	alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec3 viewPos;
};

struct Material {
	alignas(16) glm::vec3 ambient;
	alignas(16) glm::vec3 diffuse;
	alignas(16) glm::vec3 specular;
	alignas(16) glm::vec3 shininess;
};

struct DirLight {
	alignas(16) glm::vec3 direction;
	alignas(16) glm::vec3 ambient;
	alignas(16) glm::vec3 diffuse;
	alignas(16) glm::vec3 specular;
};

struct PointLight {
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 clq; // constant, linear, quadratic
	alignas(16) glm::vec3 ambient;
	alignas(16) glm::vec3 diffuse;
	alignas(16) glm::vec3 specular;
};

template<typename T> 
struct UniformBuffer
{
	void createUniformBuffer(size_t inSize, VkDevice& inDevice, VkPhysicalDevice& inPhysicalDevice, int inBufferCount = 1)
	{
		size = inSize;
		device = inDevice;
		physicalDevice = inPhysicalDevice;
		bufferCount = inBufferCount;
		type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		uniformBuffers.resize(inSize);
		uniformBufferMemory.resize(inSize);

		for (size_t i = 0; i < inSize; i++)
		{
			createBuffer(getSize() * bufferCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBufferMemory[i]);
		}

		createDescriptorBufferInfos();
	}

	void createWriteDescriptorSet(size_t index, VkDescriptorSet& DescriptorSet, std::vector<VkWriteDescriptorSet>& descriptorWrites)
	{
		VkWriteDescriptorSet DescriptorWrite;
		DescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		DescriptorWrite.pNext = nullptr;
		DescriptorWrite.dstSet = DescriptorSet;
		DescriptorWrite.dstBinding = static_cast<uint32_t>(descriptorWrites.size());
		DescriptorWrite.dstArrayElement = 0;
		DescriptorWrite.descriptorType = type;
		DescriptorWrite.descriptorCount = 1; // how many descriptor you update
		
		if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		{
			DescriptorWrite.pBufferInfo = &uniformBufferInfo[index];
		}
		else
		{
			DescriptorWrite.pImageInfo = nullptr;
		}

		descriptorWrites.emplace_back(std::move(DescriptorWrite));
	}

	void CopyData(uint32_t currentImage, std::vector<T> data)
	{
		for (int i = 0; i < bufferCount; i++)
		{
			void* virtualAddress;

			vkMapMemory(device, uniformBufferMemory[currentImage], i * getSize(), getSize(), 0, &virtualAddress);
			memcpy(virtualAddress, &data[i], getSize());
			vkUnmapMemory(device, uniformBufferMemory[currentImage]);
		}
	}

	void destroy(size_t index)
	{
		vkDestroyBuffer(device, uniformBuffers[index], nullptr);
		vkFreeMemory(device, uniformBufferMemory[index], nullptr);
		uniformBufferInfo.clear();
	}

private:
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) &&
				((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void createDescriptorBufferInfos()
	{
		for (int i = 0; i < size; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = getSize() * bufferCount;

			uniformBufferInfo.emplace_back(std::move(bufferInfo));
		}
	}

public:
	VkDeviceSize getSize()
	{
		return sizeof(T);
	}

	const VkBuffer& getUniformBuffer(int index) { return uniformBuffers[index]; }

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkDescriptorType type;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBufferMemory;
	std::vector<VkDescriptorBufferInfo> uniformBufferInfo;
	size_t size;
	int bufferCount;
};
