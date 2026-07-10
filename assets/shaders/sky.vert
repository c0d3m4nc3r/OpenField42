#version 330 core

layout(location = 0) in vec3 a_Pos;
layout(location = 2) in vec2 a_TexCoord;

layout (std140) uniform CameraBlock
{
    mat4 uView;
    mat4 uProjection;
    vec4 uViewPos;
};

out vec2 v_TexCoords;

uniform mat4 u_Model;

void main()
{
    v_TexCoords = a_TexCoord;
    mat4 skyView = mat4(mat3(uView));
    vec4 pos = uProjection * skyView * u_Model * vec4(a_Pos, 1.0);
    gl_Position = pos.xyww;
}