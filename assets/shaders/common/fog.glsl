#ifndef COMMON_FOG_GLSL
#define COMMON_FOG_GLSL

layout (std140, binding = 1) uniform FogBlock
{
    vec4 u_FogColor;
    vec4 u_FogParams; // x - start, y - end, z - enabled, w - padding
};

#ifdef VERTEX

out float v_FogCoord;

void ComputeFogCoord(vec3 eyePos)
{
    v_FogCoord = length(eyePos);
}

#endif // VERTEX

#ifdef FRAGMENT

in float v_FogCoord;

void ApplyFog(inout vec3 color)
{
    if (u_FogParams.z == 1.0)
    {
        float fogFactor = clamp((u_FogParams.y - v_FogCoord) / (u_FogParams.y - u_FogParams.x), 0.0, 1.0);
        color = mix(u_FogColor.rgb, color, fogFactor);
    }
}

#endif // FRAGMENT

#endif // COMMON_FOG_GLSL
