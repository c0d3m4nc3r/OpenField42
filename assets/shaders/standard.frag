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

layout (std140) uniform LightingBlock
{
    vec4 uDiffuseLight;
    vec4 uSpecularLight;
    vec4 uAmbientLight;
    vec4 uGlobalAmbientLight;
    vec4 uSunDirection;
};

in vec2 vTexCoords;
in vec3 vNormal;
in vec3 vFragPos;

out vec4 FragColor;

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
};

uniform Material uMaterial;

uniform bool uWireframeEnabled;
uniform vec3 uWireframeColor;

void main()
{
    if (uWireframeEnabled)
    {
        FragColor = vec4(uWireframeColor, 1.0);
        return;
    }

    vec3 objectColor = vec3(1.0, 1.0, 1.0);
    float alpha = 1.0;

    vec4 texColor = uMaterial.hasTexture ? texture(uMaterial.texture, vTexCoords) : uMaterial.diffuseColor;
    objectColor = texColor.rgb;
    alpha = texColor.a;
    
    if (uMaterial.hasDetailTexture)
    {
        vec3 detailColor = texture(uMaterial.detailTexture, vTexCoords * 128.0).rgb;
        objectColor *= detailColor * 2.0;
    }

    vec3 result;
    if (uMaterial.lighting)
    {
        vec3 ambient = ((uAmbientLight.rgb + uGlobalAmbientLight.rgb) * 2.0) * objectColor;

        vec3 norm = normalize(vNormal);
        vec3 lightDir = normalize(uSunDirection.rgb);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * uDiffuseLight.rgb * objectColor;

        vec3 specular = vec3(0.0);
        if (uMaterial.lightingSpecular)
        {
            vec3 viewDir = normalize(uViewPos - vFragPos);
            vec3 reflectDir = reflect(lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.specularPower);
            specular = spec * uSpecularLight.rgb * uMaterial.specularColor;
        }

        result = ambient + diffuse + specular;
    } 
    else 
    {
        result = objectColor;
    }

    if (uFogEnabled)
    {
        float distance = length(uViewPos - vFragPos);
        float fogFactor = clamp((uFogEnd - distance) / (uFogEnd - uFogStart), 0.0, 1.0);
        result = mix(uFogColor.rgb, result, fogFactor);
    }

    FragColor = vec4(result, alpha);
}