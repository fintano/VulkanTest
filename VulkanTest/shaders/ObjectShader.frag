#version 450

#include "input_structures.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outAlbedo;
layout(location = 3) out vec3 outArm;

void main()
{
    vec4 metallicRoughness = texture(metalRoughTex, fragTexCoord);
    float metallic = metallicRoughness.b * materialData.metal_rough_factors.r;
	float roughness = metallicRoughness.g * materialData.metal_rough_factors.g;

    outPosition = fragPos;
    outNormal = normalize(fragNormal);
    outAlbedo = texture(colorTex, fragTexCoord).rgb;
    outArm = vec3(1.0, metallic, roughness);
}