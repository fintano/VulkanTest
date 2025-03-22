#define NR_POINT_LIGHTS 4
const float PI = 3.14159265359;

struct DirLight
{
    vec3 direction;
    vec4 colorIntensity;
};

struct PointLight
{
    vec3 position;
    vec3 clq; // constant, linear, quadratic
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

vec3 CalcDirLight(DirLight light, vec3 N, vec3 V, vec3 albedo, float roughness, float metallic)
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
	return (kD * albedo / PI + specular) * radiance * NdotL;
}