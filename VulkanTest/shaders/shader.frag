#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outColorSpecular;
//layout(location = 3) out vec4 outColor;

void main()
{
    outPosition = fragPos;
    outNormal = fragNormal;
    outColorSpecular.rgb = vec3(1.0, 0.0, 0.0);
    outColorSpecular.a = 1.0;
    //outColor = vec4(1.0);
}
