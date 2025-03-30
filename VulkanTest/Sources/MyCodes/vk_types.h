#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <memory>

enum class MaterialPass : uint8_t
{
	MainColor,
	Transparent,
	Other
};

struct MaterialPipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
};

struct MaterialInstance
{
	MaterialPipeline* pipeline; // 소유권 없음
	std::vector<VkDescriptorSet> materialSet;
	MaterialPass passType;
};

struct DrawContext;

// base class for a renderable dynamic object
class IRenderable {

    virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) = 0;
};

// implementation of a drawable scene node.
// the scene node can hold children and will also keep a transform to propagate
// to them
struct Node : public IRenderable {

    // parent pointer must be a weak pointer to avoid circular dependencies
    std::weak_ptr<Node> parent;
    std::vector<std::shared_ptr<Node>> children;

    glm::mat4 localTransform;
    glm::mat4 worldTransform;

    void refreshTransform(const glm::mat4& parentMatrix);
	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) override
	{
		// draw children
		for (auto& c : children) {
			c->Draw(topMatrix, ctx);
		}
	}
};

struct GPUDrawPushConstants
{
	glm::mat4 model;
};

struct AllocatedImage
{
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;

	void Destroy(VkDevice device);
};

struct CubeMap
{
	VkImage image;
	VkDeviceMemory imageMemory;
	std::vector<std::vector<VkImageView>> imageViews;
	VkImageView cubeImageView;

	void Destroy(VkDevice device);
};