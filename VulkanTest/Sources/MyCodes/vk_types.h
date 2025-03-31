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

class DeviceWrapper {
public:
	DeviceWrapper(VkDevice device, VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkSurfaceKHR surface, GLFWwindow* window) 
	: device(device) 
	, instance(instance)
	, debugMessenger(debugMessenger)
	, surface(surface)
	, window(window)
	{}

	~DeviceWrapper();

	// 핵심: 이 변환 연산자가 VkDevice를 기대하는 모든 곳에서 
	// DeviceWrapper가 자동으로 VkDevice로 변환되도록 합니다
	operator VkDevice() const { return device; }

private:
	VkDevice device;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	GLFWwindow* window;
};

using DevicePtr = std::shared_ptr<DeviceWrapper>;

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
	AllocatedImage(DevicePtr inDevice);
	~AllocatedImage();

	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;

private:
	void Destroy();
	DevicePtr device;
};

struct CubeMap
{
	CubeMap(DevicePtr inDevice);
	~CubeMap();

	std::shared_ptr<AllocatedImage> image;
	std::vector<std::vector<VkImageView>> imageViews; // MipLevel * CubeFace

private:
	void Destroy();
	DevicePtr device;
};