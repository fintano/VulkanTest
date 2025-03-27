#include "light_structures.glsl"

layout(set = 0, binding = 0) uniform  SceneData{   
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	vec3 viewPos;
	mat4 view;
	mat4 proj;

	float exposure;
	int debugDisplayTarget;
	int padding[2];

	DirLight dirLight;
	ActivePointLights activePointLights;
} sceneData;