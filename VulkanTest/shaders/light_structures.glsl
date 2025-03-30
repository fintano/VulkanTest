#define NR_POINT_LIGHTS 4
const float PI = 3.14159265359;

struct DirLight
{
    vec3 direction;
    float _pad0;
    vec4 colorIntensity;
};

struct PointLight
{
    vec3 position;
    float _pad0;
    vec3 clq; // constant, linear, quadratic
    float _pad1;
    vec4 colorIntensity;
};

struct ActivePointLights
{
    PointLight pointLights[NR_POINT_LIGHTS];
    int activeLightMask;
};

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 CalcPointLight(PointLight light, vec3 normalVec, vec3 viewDir, vec3 fragPosition, vec3 albedoColor, float roughnessValue, float metallicValue, vec3 F0Value, float diffuseFactor, float specularFactor)
{
    // 광원을 향한 벡터
    vec3 L = normalize(light.position - fragPosition);
    // 하프 벡터
    vec3 H = normalize(viewDir + L);
    
    // 포인트 라이트의 광도 계산
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 lightColor = light.colorIntensity.rgb;
    float intensity = light.colorIntensity.a;
    vec3 radiance = lightColor * intensity * attenuation;
    
    // BRDF 계산
    float NDF = DistributionGGX(normalVec, H, roughnessValue);
    float G = GeometrySmith(normalVec, viewDir, L, roughnessValue);
    vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0Value);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normalVec, viewDir), 0.0) * max(dot(normalVec, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // kS와 kD 계산
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicValue;
    
    // 광원과 현재 위치의 각도에 따른 빛의 세기
    float NdotL = max(dot(normalVec, L), 0.0);
    
    // 최종 반환값
    return (kD * albedoColor / PI * diffuseFactor + specular * specularFactor) * radiance * NdotL;
}

vec3 CalcDirLight(DirLight light, vec3 N, vec3 V, vec3 albedo, float roughness, float metallic, float diffuseFactor, float specularFactor)
{
    vec3 F0 = vec3(0.04);
	F0      = mix(F0, albedo, metallic);

    // 광원을 향한 벡터
	vec3 L = normalize(-light.direction);
	// 하프 벡터
	vec3 H = normalize(V + L);

    vec3 lightColor = light.colorIntensity.rgb;
	float intensity = light.colorIntensity.a;
	vec3 radiance = lightColor * intensity;

	float NDF = DistributionGGX(N, H, roughness);
	float G   = GeometrySmith(N, V, L, roughness);
	vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
	vec3 specular     = numerator / denominator;

	// F는 Specular 반사율이기 때문에 kS에 사용한다.
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	// 금속이면 kD가 없다.
	kD *= 1.0 - metallic;

	// 광원과 현재 위치의 각도에 따라 빛의 세기가 달라진다.
	float NdotL = max(dot(N, L), 0.0);

	// 최종식
	return (kD * albedo / PI * diffuseFactor + specular * specularFactor) * radiance * NdotL;
}