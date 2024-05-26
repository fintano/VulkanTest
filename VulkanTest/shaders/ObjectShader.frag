#version 450

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform ColorUBO {
    vec3 objectColor;
    vec3 lightColor;
    vec3 lightPos;
    vec3 viewPos;
} colorUbo;
layout(binding = 3) uniform Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
} material;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(colorUbo.lightPos - fragPos);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (material.diffuse * diff) * colorUbo.lightColor;   

    float specularStrength = 0.5f;
    vec3 viewDir = normalize(colorUbo.viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = (material.specular * spec) * colorUbo.lightColor;  

    float ambientStrength = 0.1f;
    vec3 ambient = material.ambient * colorUbo.lightColor;

    vec3 result = (diffuse + ambient + specular) * colorUbo.objectColor;

    outColor = vec4(result, 1.0);
}
