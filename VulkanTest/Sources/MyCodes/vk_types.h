#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

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
	MaterialPipeline* pipeline;
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