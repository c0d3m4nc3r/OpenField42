#ifndef COMMON_MATERIAL_GLSL
#define COMMON_MATERIAL_GLSL

#define MAT_FLAG_HAS_DETAIL_TEX   (1 << 0) // 1
#define MAT_FLAG_LIGHTING_ENABLED (1 << 1) // 2
#define MAT_FLAG_SPECULAR_ENABLED (1 << 2) // 4
#define MAT_FLAG_IS_BILLBOARD     (1 << 3) // 8

struct Material
{
    vec4 diffuse;
    vec4 specular; // rgb - specular color, a - specular power
    int flags;
};

uniform Material u_Material;

uniform sampler2D u_MainTexture;
uniform sampler2D u_DetailTexture;

bool HasDetailTexture()
{
    return (u_Material.flags & MAT_FLAG_HAS_DETAIL_TEX) != 0;
}

bool IsLightingEnabled()
{
    return (u_Material.flags & MAT_FLAG_LIGHTING_ENABLED) != 0;
}

bool IsSpecularEnabled()
{
    return (u_Material.flags & MAT_FLAG_SPECULAR_ENABLED) != 0;
}

bool IsBillboard()
{
    return (u_Material.flags & MAT_FLAG_IS_BILLBOARD) != 0;
}

#endif // COMMON_MATERIAL_GLSL