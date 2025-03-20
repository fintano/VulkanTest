#version 450

#include "deferred.glsl"

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 fragPos = texture(position, texCoords).rgb;
    vec3 fragNormal = texture(normal, texCoords).rgb;
    vec3 albedo = texture(albedo, texCoords).rgb;
	vec3 arm = texture(arm, texCoords).rgb;
    vec3 ao = vec3(arm.r);
	float metallic = arm.b;
	float roughness = arm.g;

    vec3 N = normalize(fragNormal);
	vec3 V = normalize(sceneData.viewPos - fragPos);

	// �������� Diffuse �ݻ�� Specular �ݻ��� ������ �����ϰ� �ִ�.
	// ��ݼ��� ��ü�� �������� �ٶ���� �� 0.04 ������ �ݻ����� ���̰�,
	// �ݼ��� ���� �ݻ����� ���̸� ����(tint) ���� ��Ÿ���� ������ albedo�� F0�� ����.
	vec3 F0 = vec3(0.04);
	F0      = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
	for(int i = 0 ; i < NR_POINT_LIGHTS; i++)
    {
        if((sceneData.activePointLights.activeLightMask & (1 << i)) > 0)
        {
			// ������ ���� ����
			vec3 L = normalize(sceneData.activePointLights.pointLights[i].position - fragPos);
			// ���� ����
			vec3 H = normalize(V + L);

			// ����Ʈ ����Ʈ�� ������ ���.
			// DirectionalLight�� Directional Light ���� attenuation���� LightVector�� Constant�� ����ϸ� �ǰ�,
			// SpotLight�� �� �׿� �ش��� ������ ����ϸ� �ȴ�.
			float distance = length(sceneData.activePointLights.pointLights[i].position - fragPos);
			float attenuation = 1.0 / (distance * distance);
			vec3 radiance = sceneData.activePointLights.pointLights[i].diffuse * attenuation;

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