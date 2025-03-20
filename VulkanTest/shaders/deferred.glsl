#include "light_structures.glsl"

layout(set = 0, binding = 0) uniform  SceneData{
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	vec3 viewPos;
	mat4 view;
	mat4 proj;

	ActivePointLights activePointLights;
} sceneData;

layout(set = 1, binding = 0) uniform sampler2D position;
layout(set = 1, binding = 1) uniform sampler2D normal;
layout(set = 1, binding = 2) uniform sampler2D albedo;
layout(set = 1, binding = 3) uniform sampler2D arm;