#version 450

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform ColorUBO {
    vec3 objectColor;
    vec3 lightColor;
    //vec3 lightPos;
    vec3 viewPos;
} colorUbo;
layout(binding = 3) uniform Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 shininess;
} material;
layout(binding = 4) uniform Light {
    //vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 norm = normalize(fragNormal);
    //vec3 lightDir = normalize(light.position - fragPos);  
    vec3 lightDir = normalize(-light.direction);  
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse); 

    float specularStrength = 0.5f;
    vec3 viewDir = normalize(colorUbo.viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess.x);
    vec3 specular = light.specular * (spec * material.specular); 

    vec3 ambient = light.ambient * colorUbo.lightColor;

    vec3 result = (diffuse + ambient + specular) * colorUbo.objectColor;

    outColor = vec4(result, 1.0);
}
