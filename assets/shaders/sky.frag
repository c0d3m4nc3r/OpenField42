#version 330 core

in vec2 v_TexCoords;
out vec4 f_Color;

struct Material
{
    sampler2D texture;
    bool hasTexture;
    vec4 diffuseColor;
};

uniform Material u_Material;

uniform bool u_WireframeEnabled;

void main()
{
    if (u_WireframeEnabled)
    {
        f_Color = vec4(0.51, 0.78, 0.90, 1.0);
        return;
    }

    vec4 texColor = u_Material.hasTexture ? texture(u_Material.texture, v_TexCoords) : u_Material.diffuseColor;
    f_Color = vec4(texColor.rgb, texColor.a);
}