#include "vk_types.h"

void Node::refreshTransform(const glm::mat4& parentMatrix)
{
	worldTransform = parentMatrix * localTransform;
	for (auto c : children) {
		c->refreshTransform(worldTransform);
	}
}