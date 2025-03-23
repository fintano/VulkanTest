#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "global.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragPos;

void main()
{
    fragPos = inPosition;

    mat4 rotView = mat4(mat3(sceneData.view)); // remove translation from the view matrix
    vec4 clipPos = sceneData.proj * rotView * vec4(fragPos, 1.0);

    gl_Position = clipPos.xyww;
}