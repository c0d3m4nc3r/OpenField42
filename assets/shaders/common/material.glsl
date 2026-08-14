#ifndef COMMON_MATERIAL_GLSL
#define COMMON_MATERIAL_GLSL

// TODO: Move samplers into a separate uniform and use flags instead of booleans for better packing
struct Material
{
    sampler2D texture;
    bool hasTexture;
    
    sampler2D detailTexture;
    bool hasDetailTexture;

    vec4 diffuseColor;
    vec3 specularColor;
    float specularPower;
    
    bool lighting;
    bool lightingSpecular;
    
    bool billboard;
};

uniform Material u_Material;

#endif // COMMON_MATERIAL_GLSL