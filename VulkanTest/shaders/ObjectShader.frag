#version 450

#include "input_structures.glsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBitangent;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outAlbedo;
layout(location = 3) out vec4 outARM;

void main()
{
    outPosition = fragPos;
    
    vec3 N = normalize(fragNormal);
    
    //if (materialData.textureFlags.x > 0.5) {
     //   vec3 tangentNormal = texture(normalTex, fragTexCoord).rgb * 2.0 - 1.0;
      //  mat3 TBN = mat3(
      //      normalize(fragTangent),
      //      normalize(fragBitangent),
      //      N
       // );
     //   N = normalize(TBN * tangentNormal);
   // }
    outNormal = N * 0.5 + 0.5;

    outAlbedo = texture(colorTex, fragTexCoord).rgb * materialData.colorFactors.rgb;
    
    float roughness = 0.5;
    float metallic = 0.0;
    float ao = 1.0;
    
    if (materialData.textureFlags.y > 0.5 || materialData.textureFlags.z > 0.5) {
        // 개별 metallic/roughness 텍스처 사용
        if (materialData.textureFlags.y > 0.5) {
            metallic = texture(metallicTex, fragTexCoord).r;
        }
        
        if (materialData.textureFlags.z > 0.5) {
            roughness = texture(roughnessTex, fragTexCoord).r;
        }
    } else {
        // 이 경우는 개별 텍스처가 없을 때만 실행됨
        // metalRoughTex를 사용 (있다면)
        vec4 metallicRoughness = texture(metalRoughTex, fragTexCoord);
        roughness = metallicRoughness.g * materialData.metal_rough_factors.g;
        metallic = metallicRoughness.b * materialData.metal_rough_factors.r;
    }
    
    if (materialData.textureFlags.w > 0.5) {
        ao = texture(aoTex, fragTexCoord).r;
    }
    
    outARM = vec4(ao, roughness, metallic, 1.0);
}