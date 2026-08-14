
layout (std140, binding = 0) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_ViewPos;
};

#ifdef VERTEX // ---

layout(location = 0) in vec3 a_Pos;
layout(location = 2) in vec2 a_TexCoord;

out vec2 v_TexCoords;

uniform mat4 u_Model;

void main()
{
    v_TexCoords = a_TexCoord;
    mat4 skyView = mat4(mat3(u_View));
    vec4 pos = u_Projection * skyView * u_Model * vec4(a_Pos, 1.0);
    gl_Position = pos.xyww;
}
#endif // VERTEX

#ifdef FRAGMENT // ---

in vec2 v_TexCoords;
out vec4 f_Color;

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
#endif // FRAGMENT