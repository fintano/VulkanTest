#version 450

#include "deferred.glsl"

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 FragColor;

void main()
{
    vec3 WorldPos = texture(position, texCoords).rgb;
    vec3 N = texture(normal, texCoords).rgb;
    N = N * 2.0 - 1.0;
    vec3 albedo = pow(texture(albedo, texCoords).rgb, vec3(2.2));
	vec3 arm = texture(arm, texCoords).rgb;
    vec3 ao = vec3(arm.r);
	float metallic = arm.b;
	float roughness = arm.g;

    if(sceneData.debugDisplayTarget > 0)
	{
		switch (sceneData.debugDisplayTarget) {
		case 1:
			FragColor = vec4(WorldPos, 1.0);
			return;
		case 2:
			FragColor = vec4(N, 1.0);
			return;
		case 3:
			FragColor = vec4(texCoords, 1.0 , 1.0);
			return;
		case 4:
			FragColor = vec4(ao, 1.0);
			return;
		case 5:
			FragColor = vec4(roughness, roughness, roughness, 1.0);
			return;
		case 6:
			FragColor = vec4(metallic, metallic, metallic, 1.0);
			return;
		}
	}

    // input lighting data
    //vec3 N = getNormalFromMap();
    vec3 V = normalize(sceneData.viewPos - WorldPos);
    vec3 R = reflect(-V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(sceneData.activePointLights.pointLights[i].position - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(sceneData.activePointLights.pointLights[i].position - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = vec3(sceneData.activePointLights.pointLights[i].colorIntensity) * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	                
            
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(diffuseMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(N, 1.0);
}