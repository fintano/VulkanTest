#version 450

#include "input_structures.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main()
{
    float lightValue = max(dot(fragNormal, sceneData.sunlightDirection.xyz), 0.1f);

	vec3 color = fragColor * texture(colorTex,fragTexCoord).xyz;
	vec3 ambient = color *  sceneData.ambientColor.xyz;

	outColor = vec4(color * lightValue *  sceneData.sunlightColor.w + ambient ,1.0f);
}
