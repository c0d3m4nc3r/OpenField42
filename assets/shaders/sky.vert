#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoord;

layout (std140) uniform CameraBlock
{
    mat4 uView;
    mat4 uProjection;
    vec4 uViewPos;
};

out vec2 vTexCoords;

uniform mat4 uModel;

void main()
{
    vTexCoords = aTexCoord;
    mat4 skyView = mat4(mat3(uView));
    vec4 pos = uProjection * skyView * uModel * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}