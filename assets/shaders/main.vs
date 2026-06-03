#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout (std140) uniform CameraBlock
{
    mat4 uView;
    mat4 uProjection;
    vec3 uViewPos;
};

out vec2 vTexCoords;
out vec3 vNormal;
out vec3 vFragPos;

uniform mat4 uModel;

uniform bool uIsSky;

void main()
{
    vTexCoords = aTexCoord;
    
    if (uIsSky) {
        mat4 skyView = mat4(mat3(uView));
        vec4 pos = uProjection * skyView * uModel * vec4(aPos, 1.0);
        gl_Position = pos.xyww;
    } else {
        vFragPos = vec3(uModel * vec4(aPos, 1.0));
        vNormal = mat3(transpose(inverse(uModel))) * aNormal;
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    }
}