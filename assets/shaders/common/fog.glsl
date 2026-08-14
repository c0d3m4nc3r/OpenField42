#ifndef COMMON_FOG_GLSL
#define COMMON_FOG_GLSL

layout (std140, binding = 1) uniform FogBlock
{
    vec4 u_FogColor;
    vec4 u_FogParams; // x - start, y - end, z - enabled, w - padding
};

void ApplyFog(inout vec3 color, float distance)
{
    if (u_FogParams.z == 1.0)
    {
        float fogFactor = clamp((u_FogParams.y - distance) / (u_FogParams.y - u_FogParams.x), 0.0, 1.0);
        color = mix(u_FogColor.rgb, color, fogFactor);
    }
}

#endif // COMMON_FOG_GLSL