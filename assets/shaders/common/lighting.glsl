#ifndef COMMON_LIGHTING_GLSL
#define COMMON_LIGHTING_GLSL

#include "common/material.glsl"

layout (std140, binding = 2) uniform LightingBlock
{
    vec4 u_DiffuseLight;
    vec4 u_SpecularLight;
    vec4 u_AmbientLight;
    vec4 u_GlobalAmbientLight;
    vec4 u_SunDirection;
};

vec3 CalcDirLight(vec3 fragPos, vec3 normal, vec3 viewPos, vec3 albedo)
{
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_SunDirection.rgb);
    
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 totalDiffuse = (u_AmbientLight.rgb + u_GlobalAmbientLight.rgb) + (diff * u_DiffuseLight.rgb);
    
    vec3 baseColor = clamp(albedo * totalDiffuse * 2.0, 0.0, 1.0);

    vec3 specular = vec3(0.0);
    if (IsSpecularEnabled())
    {
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.specular.a);
        specular = spec * u_SpecularLight.rgb * u_Material.specular.rgb;
    }

    return baseColor + specular;
}

#endif // COMMON_LIGHTING_GLSL