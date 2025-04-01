#include "global.glsl"

layout(set = 1, binding = 0) uniform GLTFMaterialData{   

	vec4 colorFactors;
	vec4 metal_rough_factors; // x=metallicFactor, y=roughnessFactor, z= normalScale, a=occulusionStrength
	vec4 textureFlags; // x=useNormalMap, y=useMetallicMap, z=useRoughnessMap, w=useAOMap
	vec4 emissiveFactors;
	
} materialData;

layout(set = 1, binding = 1) uniform sampler2D colorTex;
layout(set = 1, binding = 2) uniform sampler2D metalRoughTex;
layout(set = 1, binding = 3) uniform sampler2D normalTex;
layout(set = 1, binding = 4) uniform sampler2D metallicTex;  
layout(set = 1, binding = 5) uniform sampler2D roughnessTex;
layout(set = 1, binding = 6) uniform sampler2D aoTex;
layout(set = 1, binding = 7) uniform sampler2D emissiveTex;