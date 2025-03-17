#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "input_structures.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;

//push constants block
layout( push_constant ) uniform constants
{
	mat4 render_matrix;
	//VertexBuffer vertexBuffer;
} PushConstants;

void main() 
{
	//Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	
	vec4 position = vec4(inPosition, 1.0f);

	gl_Position =  sceneData.viewproj * PushConstants.render_matrix *position;

	outNormal = (PushConstants.render_matrix * vec4(inNormal, 0.f)).xyz;
	outColor = inColor.xyz * materialData.colorFactors.xyz;	
	outUV = inTexCoord;
}