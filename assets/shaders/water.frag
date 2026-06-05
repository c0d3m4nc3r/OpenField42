#version 330 core

layout (std140) uniform CameraBlock
{
    mat4 uView;
    mat4 uProjection;
    vec3 uViewPos;
};

layout (std140) uniform FogBlock
{
    vec4 uFogColor;
    float uFogStart;
    float uFogEnd;
    bool uFogEnabled;
};

layout(std140) uniform WaterBlock
{
    vec4 uScroll1; // xy = dir, z = speed, w = padding
    vec4 uScroll2; // xy = dir, z = speed, w = padding
};

in vec2 vTexCoords;
in vec3 vNormal;
in vec3 vFragPos;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D uTexLayer1;
uniform sampler2D uTexLayer2;

uniform float uTime;
uniform vec3 uSunLightDir;

uniform bool uWireframeMode;

void main()
{
    if (uWireframeMode)
    {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
        return;
    }

    vec2 uv1 = vTexCoords + uScroll1.xz * uScroll1.z * uTime;
    vec2 uv2 = vTexCoords + uScroll2.xz * uScroll2.z * uTime;

    vec4 layerColor1 = texture(uTexLayer1, uv1);
    vec4 layerColor2 = texture(uTexLayer2, uv2);
    
    vec4 objectColor = (layerColor1 + layerColor2) * vColor;

    vec3 finalColor = objectColor.rgb;

    if (uFogEnabled)
    {
        float distance = length(uViewPos - vFragPos);
        float fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);
        
        finalColor = mix(uFogColor.rgb, finalColor, fogFactor);
    }

    FragColor = vec4(finalColor, vColor.a);
}