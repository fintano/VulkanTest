#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <string>
#include <unordered_map>
#include <stdexcept>

#include "vk_types.h"

struct MaterialPipeline;
class VulkanTutorialExtension;
struct VertexOnlyPos;

class SimplePipeline
{
public:
	enum class RenderType
	{
		forward,
		deferred
	};

private:
	// ������ ������ �����ϱ� ���� Ÿ�� ���� ����
	class TypeSizeTracker {
	public:
		template<typename T>
		static size_t getSize() {
			return sizeof(T);
		}

		template<typename T>
		static void registerType(uint32_t binding);

		static size_t getRegisteredSize(uint32_t binding);

	private:
		static std::unordered_map<uint32_t, size_t> typeSizes;
	};

	// ��ũ���� ���� ���� Ŭ����
	class DescriptorBuilder {
	private:
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkDescriptorType> descriptorTypes;
		uint32_t currentBinding = 0;

	public:
		// �ؽ�ó �߰�
		DescriptorBuilder& addTexture(VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT);

		// ���ø�ȭ�� ������ ���� �߰�
		template<typename T>
		DescriptorBuilder& addUniformBuffer(VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT) {
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = currentBinding;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			binding.descriptorCount = 1;
			binding.stageFlags = stageFlags;
			bindings.push_back(binding);
			descriptorTypes.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

			// Ÿ�� ũ�� ���
			TypeSizeTracker::registerType<T>(currentBinding);

			currentBinding++;
			return *this;
		}

		// ���ø�ȭ�� ���丮�� ���� �߰�
		template<typename T>
		DescriptorBuilder& addStorageBuffer(VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT) {
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = currentBinding;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			binding.descriptorCount = 1;
			binding.stageFlags = stageFlags;
			bindings.push_back(binding);
			descriptorTypes.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

			// Ÿ�� ũ�� ���
			TypeSizeTracker::registerType<T>(currentBinding);

			currentBinding++;
			return *this;
		}

		// ��ũ���� ���̾ƿ� ����
		VkDescriptorSetLayout buildLayout(VkDevice device) const;

		// Ư�� ���ε��� ��ũ���� Ÿ�� ��������
		VkDescriptorType getDescriptorType(uint32_t binding) const;

		// ���ε� ���� ��ȯ
		size_t getBindingCount() const;

		// ��� ���̾ƿ� ���ε� ��������
		const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const;
	};

	VkDescriptorSetLayout layout;
	MaterialPipeline pipeline;
	VkDescriptorPool descriptorPool;  // ��ũ���� Ǯ ���� (������ ����)
	std::vector<VkDescriptorSet> descriptorSets; // ���������� ��ũ���� �� ����
	std::vector<VkDescriptorSetLayout> externalLayouts; // �ܺ� ���̾ƿ� ����
	std::vector<VkDescriptorSet> externalDescriptorSets;

protected:
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	std::vector<VkPushConstantRange> matrixRanges;
	DescriptorBuilder descriptorBuilder;

public:
	RenderType renderType;
	std::string vertShaderPath, fragShaderPath;
	VkExtent2D viewportExtent;
	int swapchainImageNum;

	SimplePipeline(VkExtent2D viewportExtent, std::string vertShaderPath, std::string fragShaderPath,
		RenderType renderType, VkDescriptorPool pool, int swapchainImageNum);

	// ��ũ���� ���� ��������
	DescriptorBuilder& getDescriptorBuilder();

	virtual void createDescriptorSetLayout(VkDevice device);
	virtual void createVertexInput();

	void addExternalDescriptorSetLayout(VkDescriptorSetLayout externalLayout);
	void addExternalDescriptorSet(VkDescriptorSet externalDescriptorSet);
	void addExternalDescriptorSet(const std::vector<VkDescriptorSet>& externalDescriptorSet);

	// ��ũ���� �� �Ҵ�
	void allocateDescriptorSet(VkDevice device);
	void buildPipeline(VulkanTutorialExtension* engine, std::function<void(VkGraphicsPipelineCreateInfo&)> modifyFunc = nullptr);

	// �ؽ�ó ��ũ���� ������Ʈ
	void updateTextureDescriptor(VkDevice device, uint32_t index, uint32_t binding,
		VkImageView imageView, VkSampler sampler,
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// ���� ��ũ���� ������Ʈ (�ڵ� ũ��)
	void updateBufferDescriptor(VkDevice device, uint32_t index, uint32_t binding,
		VkBuffer buffer, VkDeviceSize offset = 0);

	// ���� ��ũ���� ������Ʈ (����� ũ��)
	void updateBufferDescriptor(VkDevice device, uint32_t index, uint32_t binding,
		VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

	// ���� SimplePipeline ȣȯ �޼ҵ�
	void writeDescriptor();

	void cleanup(VkDevice device);

	std::shared_ptr<MaterialInstance> makeMaterial();
	MaterialPipeline& getMaterialPipeline() { return pipeline; }
};

class SimplePipelinePosOnly : public SimplePipeline
{
public:
	SimplePipelinePosOnly(VkExtent2D viewportExtent, std::string vertShaderPath, std::string fragShaderPath,
		RenderType renderType, VkDescriptorPool pool, int swapchainImageNum)
		:SimplePipeline(viewportExtent, vertShaderPath, fragShaderPath, renderType, pool, swapchainImageNum)
	{
	}

private:
	virtual void createVertexInput() override;
};