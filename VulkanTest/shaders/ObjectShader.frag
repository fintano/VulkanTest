#version 450

#define NR_POINT_LIGHTS 4

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;
    vec3 clq; // constant, linear, quadratic
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

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

layout(binding = 4) uniform dirLightUniform {
    DirLight dirLight;
};

layout(binding = 5) uniform pointLightsUniform {
    PointLight pointLights[NR_POINT_LIGHTS];
    int activeLightMask;
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(colorUbo.viewPos - fragPos);

    // phase 1: Directinal lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: Point lights
    for(int i = 0 ; i < NR_POINT_LIGHTS; i++)
    {
        if((activeLightMask & (1 << i)) > 0)
        {
            result += CalcPointLight(pointLights[i], norm, fragPos, viewDir);
        }
    }
    // phase 3: Spot Light 
    // 
    
    outColor = vec4(result, 1.0) * texture(texSampler, fragTexCoord);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);  
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess.x);
    // combine results
    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    return (diffuse + ambient + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess.x);
    // attenuation
    float constant = light.clq.x;
    float linear = light.clq.y;
    float quadratic = light.clq.z;
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));  

    // combine results
    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (diffuse + ambient + specular);
}