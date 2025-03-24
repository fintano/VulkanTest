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
	// 사이즈 정보를 추적하기 위한 타입 매핑 헬퍼
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

	// 디스크립터 빌더 내부 클래스
	class DescriptorBuilder {
	private:
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkDescriptorType> descriptorTypes;
		uint32_t currentBinding = 0;

	public:
		// 텍스처 추가
		DescriptorBuilder& addTexture(VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT);

		// 템플릿화된 유니폼 버퍼 추가
		template<typename T>
		DescriptorBuilder& addUniformBuffer(VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT) {
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = currentBinding;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			binding.descriptorCount = 1;
			binding.stageFlags = stageFlags;
			bindings.push_back(binding);
			descriptorTypes.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

			// 타입 크기 등록
			TypeSizeTracker::registerType<T>(currentBinding);

			currentBinding++;
			return *this;
		}

		// 템플릿화된 스토리지 버퍼 추가
		template<typename T>
		DescriptorBuilder& addStorageBuffer(VkShaderStageFlags stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT) {
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = currentBinding;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			binding.descriptorCount = 1;
			binding.stageFlags = stageFlags;
			bindings.push_back(binding);
			descriptorTypes.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

			// 타입 크기 등록
			TypeSizeTracker::registerType<T>(currentBinding);

			currentBinding++;
			return *this;
		}

		// 디스크립터 레이아웃 생성
		VkDescriptorSetLayout buildLayout(VkDevice device) const;

		// 특정 바인딩의 디스크립터 타입 가져오기
		VkDescriptorType getDescriptorType(uint32_t binding) const;

		// 바인딩 개수 반환
		size_t getBindingCount() const;

		// 상수 레이아웃 바인딩 가져오기
		const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const;
	};

	VkDescriptorSetLayout layout;
	MaterialPipeline pipeline;
	VkDescriptorPool descriptorPool;  // 디스크립터 풀 참조 (소유권 없음)
	std::vector<VkDescriptorSet> descriptorSets; // 내부적으로 디스크립터 셋 관리
	std::vector<VkDescriptorSetLayout> externalLayouts; // 외부 레이아웃 저장
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

	// 디스크립터 빌더 가져오기
	DescriptorBuilder& getDescriptorBuilder();

	virtual void createDescriptorSetLayout(VkDevice device);
	virtual void createVertexInput();

	void addExternalDescriptorSetLayout(VkDescriptorSetLayout externalLayout);
	void addExternalDescriptorSet(VkDescriptorSet externalDescriptorSet);
	void addExternalDescriptorSet(const std::vector<VkDescriptorSet>& externalDescriptorSet);

	// 디스크립터 셋 할당
	void allocateDescriptorSet(VkDevice device);
	void buildPipeline(VulkanTutorialExtension* engine, std::function<void(VkGraphicsPipelineCreateInfo&)> modifyFunc = nullptr);

	// 텍스처 디스크립터 업데이트
	void updateTextureDescriptor(VkDevice device, uint32_t index, uint32_t binding,
		VkImageView imageView, VkSampler sampler,
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// 버퍼 디스크립터 업데이트 (자동 크기)
	void updateBufferDescriptor(VkDevice device, uint32_t index, uint32_t binding,
		VkBuffer buffer, VkDeviceSize offset = 0);

	// 버퍼 디스크립터 업데이트 (명시적 크기)
	void updateBufferDescriptor(VkDevice device, uint32_t index, uint32_t binding,
		VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

	// 기존 SimplePipeline 호환 메소드
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