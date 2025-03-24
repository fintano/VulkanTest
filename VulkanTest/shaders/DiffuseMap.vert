#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "input_structures.glsl"

layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pushConstants;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragPos;

void main()
{
    fragPos = inPosition;
    gl_Position = pushConstants.mvp * vec4(inPosition, 1.0);
}