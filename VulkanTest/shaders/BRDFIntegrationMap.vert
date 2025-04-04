#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;

layout(location = 0) out vec2 fragTex;

void main()
{
    gl_Position = vec4(inPos, 1.0);
 
    fragTex = inTex;
}