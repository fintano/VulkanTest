#include "global.glsl"

layout(set = 1, binding = 0) uniform sampler2D position;
layout(set = 1, binding = 1) uniform sampler2D normal;
layout(set = 1, binding = 2) uniform sampler2D albedo;
layout(set = 1, binding = 3) uniform sampler2D arm;
layout(set = 1, binding = 4) uniform samplerCube diffuseMap;
layout(set = 1, binding = 5) uniform samplerCube prefilterMap;
layout(set = 1, binding = 6) uniform sampler2D brdfLUT;
layout(set = 1, binding = 7) uniform sampler2D emissive;