#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform samplerCube cubeMap;

layout(location = 0) in vec3 localPos;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 envColor = texture(cubeMap, localPos).rgb;
    
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
  
    outColor = vec4(envColor, 1.0);
}