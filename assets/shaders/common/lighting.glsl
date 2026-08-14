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

vec3 calcDirLight(vec3 fragPos, vec3 normal, vec3 viewPos, vec3 albedo, Material mat)
{
    vec3 ambient = (u_AmbientLight.rgb + u_GlobalAmbientLight.rgb) * 2.0 * albedo;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(u_SunDirection.rgb);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_DiffuseLight.rgb * albedo;

    vec3 specular = vec3(0.0);
    if (mat.lightingSpecular)
    {
        vec3 viewDir = normalize(viewPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.specularPower);
        specular = spec * u_SpecularLight.rgb * mat.specularColor;
    }

    return ambient + diffuse + specular;
}

#endif // COMMON_LIGHTING_GLSL