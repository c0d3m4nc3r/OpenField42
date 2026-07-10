#version 330 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 4) in vec2 a_SpriteOfs;

layout (std140) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_ViewPos;
};

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 u_Model;

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

void main()
{
    v_TexCoords = a_TexCoord;

    if (u_Material.billboard) 
    {
        vec3 worldPos = vec3(u_Model * vec4(a_Pos, 1.0));

        vec3 camRight = vec3(u_View[0][0], u_View[1][0], u_View[2][0]);
        vec3 camUp    = vec3(u_View[0][1], u_View[1][1], u_View[2][1]);

        worldPos += camRight * a_SpriteOfs.x;
        worldPos += camUp    * a_SpriteOfs.y;

        v_FragPos = worldPos;
        v_Normal = -vec3(u_View[0][2], u_View[1][2], u_View[2][2]);
    }
    else 
    {
        v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
        v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    }

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}
