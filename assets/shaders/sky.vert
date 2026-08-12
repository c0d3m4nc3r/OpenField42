#version 450 core

layout(location = 0) in vec3 a_Pos;
layout(location = 2) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_ViewPos;
};

out vec2 v_TexCoords;

uniform mat4 u_Model;

void main()
{
    v_TexCoords = a_TexCoord;
    mat4 skyView = mat4(mat3(u_View));
    vec4 pos = u_Projection * skyView * u_Model * vec4(a_Pos, 1.0);
    gl_Position = pos.xyww;
}