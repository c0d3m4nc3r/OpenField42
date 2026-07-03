#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 4) in vec2 aSpriteOfs;

layout (std140) uniform CameraBlock
{
    mat4 uView;
    mat4 uProjection;
    vec4 uViewPos;
};

out vec2 vTexCoords;
out vec3 vNormal;
out vec3 vFragPos;

uniform mat4 uModel;

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

uniform Material uMaterial;

void main()
{
    vTexCoords = aTexCoord;

    if (uMaterial.billboard) 
    {
        vec3 worldPos = vec3(uModel * vec4(aPos, 1.0));

        vec3 camRight = vec3(uView[0][0], uView[1][0], uView[2][0]);
        vec3 camUp    = vec3(uView[0][1], uView[1][1], uView[2][1]);

        worldPos += camRight * aSpriteOfs.x;
        worldPos += camUp    * aSpriteOfs.y;

        vFragPos = worldPos;
        vNormal = -vec3(uView[0][2], uView[1][2], uView[2][2]);
    }
    else 
    {
        vFragPos = vec3(uModel * vec4(aPos, 1.0));
        vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    }

    gl_Position = uProjection * uView * vec4(vFragPos, 1.0);
}
