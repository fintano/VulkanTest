#version 450

#include "input_structures.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 albedo = texture(colorTex, fragTexCoord).rgb;
	vec4 metallicRoughness = texture(metalRoughTex, fragTexCoord);
	float metallic = metallicRoughness.b * materialData.metal_rough_factors.r;
	float roughness = metallicRoughness.g * materialData.metal_rough_factors.g;
	vec3 ao = vec3(1.0);

	vec3 N = normalize(fragNormal);
	vec3 V = normalize(sceneData.viewPos - fragPos);

	// 프리넬은 Diffuse 반사와 Specular 반사의 비율을 내포하고 있다.
	// 비금속은 물체를 수직으로 바라봤을 때 0.04 정도의 반사율을 보이고,
	// 금속은 높은 반사율을 보이며 색상(tint) 또한 나타나기 때문에 albedo를 F0로 쓴다.
	vec3 F0 = vec3(0.04);
	F0      = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for(int i = 0 ; i < NR_POINT_LIGHTS; i++)
    {
        if((sceneData.activePointLights.activeLightMask & (1 << i)) > 0)
        {
			// 광원을 향한 벡터
			vec3 L = normalize(sceneData.activePointLights.pointLights[i].position - fragPos);
			// 하프 벡터
			vec3 H = normalize(V + L);

			// 포인트 라이트의 광도를 계산.
			// DirectionalLight면 Directional Light 같이 attenuation없고 LightVector는 Constant로 계산하면 되고,
			// SpotLight면 또 그에 해당한 식으로 계산하면 된다.
			float distance = length(sceneData.activePointLights.pointLights[i].position - fragPos);
			float attenuation = 1.0 / (distance * distance);
			vec3 lightColor = sceneData.activePointLights.pointLights[i].colorIntensity.rgb;
			float intensity = sceneData.activePointLights.pointLights[i].colorIntensity.a;
			vec3 radiance = lightColor * intensity * attenuation;

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
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }
    }

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color   = ambient + Lo;

	// tone mapping and gamma correction
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));

	outColor = vec4(color, 1.0);
}
