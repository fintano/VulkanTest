#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "input_structures.glsl"

//push constants block
layout( push_constant ) uniform constants
{
	mat4 model;
} PushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec3 fragTangent;
layout(location = 5) out vec3 fragBitangent;

void main()
{
    gl_Position = sceneData.proj * sceneData.view * PushConstants.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragNormal = mat3(transpose(inverse(PushConstants.model))) * inNormal;
    fragPos = vec3(PushConstants.model * vec4(inPosition, 1.0));
    fragTangent = mat3(PushConstants.model) * inTangent;
    fragBitangent = mat3(PushConstants.model) * inBitangent;
}