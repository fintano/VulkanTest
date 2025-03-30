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
    // ������ ���� ����
    vec3 L = normalize(light.position - fragPosition);
    // ���� ����
    vec3 H = normalize(viewDir + L);
    
    // ����Ʈ ����Ʈ�� ���� ���
    float distance = length(light.position - fragPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 lightColor = light.colorIntensity.rgb;
    float intensity = light.colorIntensity.a;
    vec3 radiance = lightColor * intensity * attenuation;
    
    // BRDF ���
    float NDF = DistributionGGX(normalVec, H, roughnessValue);
    float G = GeometrySmith(normalVec, viewDir, L, roughnessValue);
    vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0Value);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normalVec, viewDir), 0.0) * max(dot(normalVec, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // kS�� kD ���
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallicValue;
    
    // ������ ���� ��ġ�� ������ ���� ���� ����
    float NdotL = max(dot(normalVec, L), 0.0);
    
    // ���� ��ȯ��
    return (kD * albedoColor / PI * diffuseFactor + specular * specularFactor) * radiance * NdotL;
}

vec3 CalcDirLight(DirLight light, vec3 N, vec3 V, vec3 albedo, float roughness, float metallic, float diffuseFactor, float specularFactor)
{
    vec3 F0 = vec3(0.04);
	F0      = mix(F0, albedo, metallic);

    // ������ ���� ����
	vec3 L = normalize(-light.direction);
	// ���� ����
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

	// F�� Specular �ݻ����̱� ������ kS�� ����Ѵ�.
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;

	// �ݼ��̸� kD�� ����.
	kD *= 1.0 - metallic;

	// ������ ���� ��ġ�� ������ ���� ���� ���Ⱑ �޶�����.
	float NdotL = max(dot(N, L), 0.0);

	// ������
	return (kD * albedo / PI * diffuseFactor + specular * specularFactor) * radiance * NdotL;
}