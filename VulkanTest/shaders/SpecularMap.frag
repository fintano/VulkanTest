#version 450

#include "light_structures.glsl"

layout(location = 0) in vec3 localPos;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform samplerCube environmentMap;

layout(push_constant) uniform PushConstants {
    layout (offset = 64) float roughness;
} pushConstants;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}  

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}  

// ∫ Li(p,ωi)dωi, 단순히 Prefiltered 맵만 생성. 스페큘러 BRDF는 아직 사용하지 않았다. 
void main()
{		
    vec3 N = normalize(localPos);    
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;   
    vec3 prefilteredColor = vec3(0.0);     
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // 랜덤하지만 균등한 표본 검출, 큰 수의 법칙에 따라 샘플을 많이하면 정확도가 올라간다. 
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        // When it comes to the microsurface model, 
        // we can imagine the specular lobe as the reflection orientation about the microfacet halfway vectors 
        // given some incoming light direction. Seeing as most light rays end up in a specular lobe 
        // reflected around the microfacet halfway vectors, 
        // H 벡터를 roughness 값에 따라 뽑아 내는 과정 
        vec3 H  = ImportanceSampleGGX(Xi, N, pushConstants.roughness);
        // H로 빛의 방향, 세기를 구한다. 
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        // 이 빛의 세기를 누적해서 평균을 낸다. 그럼 얼마나 빛이 이 지점에 들어왔는지 알 수 있다?
        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(N, H, pushConstants.roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = pushConstants.roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;

    outColor = vec4(prefilteredColor, 1.0);
}