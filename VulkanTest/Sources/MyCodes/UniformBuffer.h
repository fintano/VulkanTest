#pragma once

#include <stdexcept>
#include <iostream>

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

		data.resize(inBufferCount);

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

	void CopyData(uint32_t currentImage = 0)
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
		if (size != 1 || (size == 1 && index == 0))
		{
			vkDestroyBuffer(device, uniformBuffers[index], nullptr);
			vkFreeMemory(device, uniformBufferMemory[index], nullptr);
			uniformBufferInfo.clear();
		}
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
		else
		{
			std::cout << "Uniform Buffer VkBuffer " << buffer << " " << std::endl;
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

	const VkBuffer& getUniformBuffer(int index = 0) { return uniformBuffers[index]; }

	const std::vector<T>& getData() const { return data; }
	std::vector<T>& getData() { return data; }

	T& getFirstInstanceData()
	{
		if (data.empty())
		{
			data.resize(bufferCount);
		}
		return data[0];
	}

	T& clearAndGetFirstInstanceData()
	{
		data.clear();
		return getFirstInstanceData();
	}

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkDescriptorType type;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBufferMemory;
	std::vector<VkDescriptorBufferInfo> uniformBufferInfo;
	size_t size;
	int bufferCount;

	// the data to copy to gpu.
	std::vector<T> data;
};
