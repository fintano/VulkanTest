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

layout(std140,binding = 0) uniform ColorUBO {
    vec3 objectColor;
    vec3 lightColor;
    //vec3 lightPos;
    vec3 viewPos;
    uvec4 debug;
} colorUbo;

layout(binding = 1) uniform dirLightUniform {
    DirLight dirLight;
};

layout(binding = 2) uniform pointLightsUniform {
    PointLight pointLights[NR_POINT_LIGHTS];
    int activeLightMask;
};

layout(binding = 3) uniform sampler2D position;
layout(binding = 4) uniform sampler2D normal;
layout(binding = 5) uniform sampler2D colorSpecular;

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float specular);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specular);

void main()
{
    uint displayDebugTarget = colorUbo.debug.x;
    if(displayDebugTarget == 1)
    {
         // 텍스처 좌표를 2배로 키움 (사분할 화면)
        vec2 quadTexCoords = texCoords * 2.0;

        // 각 사분면을 계산
         vec3 fragPos_Debug = texture(position, quadTexCoords).rgb;
         vec3 fragNormal_Debug = texture(normal, quadTexCoords).rgb;
         vec3 fragColor_Debug = texture(colorSpecular, quadTexCoords).rgb;
         vec3 fragSpecular_Debug = texture(colorSpecular, quadTexCoords).aaa;

        // 사분면마다 다른 계산을 수행
        // 사분면을 구분하는 수학적 표현
        float xInFirstHalf = step(quadTexCoords.x, 1.0);
        float yInFirstHalf = step(quadTexCoords.y, 1.0);
    
        // 각 사분면에 대해 텍스처 좌표 계산
        fragPos_Debug *= xInFirstHalf * yInFirstHalf;
        fragNormal_Debug *= xInFirstHalf * (1.0 - yInFirstHalf);
        fragColor_Debug *= (1.0 - xInFirstHalf) * yInFirstHalf;
        fragSpecular_Debug *= (1.0 - xInFirstHalf) * (1.0 - yInFirstHalf);

        outColor = vec4(fragPos_Debug + fragNormal_Debug + fragColor_Debug + fragSpecular_Debug, 1.0);
    }
    else
    {
        vec3 fragPos = texture(position, texCoords).rgb;
        vec3 fragNormal = texture(normal, texCoords).rgb;
        vec4 fragColorSpecular = texture(colorSpecular, texCoords);
        vec3 fragColor = fragColorSpecular.rgb;
        float fragSpecular = fragColorSpecular.a;

        vec3 norm = normalize(fragNormal);
        vec3 viewDir = normalize(colorUbo.viewPos - fragPos);

        // phase 1: Directinal lighting
        vec3 result = CalcDirLight(dirLight, norm, viewDir, fragColor, fragSpecular);
        // phase 2: Point lights
        for(int i = 0 ; i < NR_POINT_LIGHTS; i++)
        {
            if((activeLightMask & (1 << i)) > 0)
            {
                result += CalcPointLight(pointLights[i], norm, fragPos, viewDir, fragColor, fragSpecular);
           }
        }
        // phase 3: Spot Light 
     

         outColor = vec4(result, 1.0);
     }
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float specular)
{
    vec3 lightDir = normalize(-light.direction);  
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), /*material.shininess.x*/ 32.0);
    // combine results
    vec3 resultAmbient = light.ambient * albedo;
    vec3 resultDiffuse = light.diffuse * diff * albedo;
    vec3 resultSpecular = light.specular * spec * specular;
    return (resultDiffuse + resultAmbient + resultSpecular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float specular)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), /*material.shininess.x*/ 32.0);
    // attenuation
    float constant = light.clq.x;
    float linear = light.clq.y;
    float quadratic = light.clq.z;
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));  

    // combine results
    vec3 resultAmbient = light.ambient * albedo;
    vec3 resultDiffuse = light.diffuse * diff * albedo;
    vec3 resultSpecular = light.specular * spec * specular;
    resultAmbient *= attenuation;
    resultDiffuse *= attenuation;
    resultSpecular *= attenuation;
    return (resultDiffuse + resultAmbient + resultSpecular);
}