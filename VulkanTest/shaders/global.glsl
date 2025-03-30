#include "light_structures.glsl"

layout(set = 0, binding = 0) uniform  SceneData{   
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
	vec3 viewPos;
	float _pad0;
	mat4 view;
	mat4 proj;

	vec4 exposureDisplay;

	DirLight dirLight;
	ActivePointLights activePointLights;
} sceneData;