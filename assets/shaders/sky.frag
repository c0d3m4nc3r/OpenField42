#version 330 core

in vec2 vTexCoords;
out vec4 FragColor;

struct Material
{
    sampler2D texture;
    bool hasTexture;
    vec4 diffuseColor;
};

uniform Material uMaterial;

uniform bool uWireframeMode;

void main()
{
    if (uWireframeMode)
    {
        FragColor = vec4(0.5, 0.7, 1.0, 1.0);
        return;
    }

    vec4 texColor = uMaterial.hasTexture ? texture(uMaterial.texture, vTexCoords) : uMaterial.diffuseColor;
    FragColor = vec4(texColor.rgb, texColor.a);
}