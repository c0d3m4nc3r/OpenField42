#version 330 core

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

struct Fog
{
    vec3 color;
    float start;
    float end;
    bool enabled;
};

struct Lighting
{
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    vec3 globalAmbient;
};

struct Water
{
    sampler2D texLayer1;
    sampler2D texLayer2;
    vec2 scrollDir1;
    vec2 scrollDir2;
    float scrollSpeed1;
    float scrollSpeed2;

    vec4 deepColor;
    vec4 shallowColor;
    float alphaDepth;

    sampler2D depthMap;
    float minDepth;
    float maxDepth;
};

uniform Material uMaterial;
uniform Fog uFog;
uniform Lighting uLighting;
uniform Water uWater;

uniform vec3 uViewPos;
uniform float uTime;

uniform vec3 uSunLightDir;

uniform bool uWireframeMode;
uniform vec3 uWireframeColor;

uniform bool uIsSky;
uniform bool uIsWater;

const float worldSize = 2048.0;

void main()
{
    if (uWireframeMode)
    {
        FragColor = vec4(uWireframeColor, 1.0);
        return;
    }

    vec3 objectColor = vec3(1.0, 1.0, 1.0);
    float alpha = 1.0;

    if (uIsWater)
    {
        vec2 uv1 = vTexCoords + uWater.scrollDir1 * uWater.scrollSpeed1 * uTime;
        vec2 uv2 = vTexCoords + uWater.scrollDir2 * uWater.scrollSpeed2 * uTime;

        vec3 layerColor1 = texture(uWater.texLayer1, uv1).rgb;
        vec3 layerColor2 = texture(uWater.texLayer2, uv2).rgb;

        vec3 waterColor = layerColor1 + layerColor2;

        objectColor = waterColor;
        alpha = 0.5;

    } else {
        vec4 texColor = uMaterial.hasTexture ? texture(uMaterial.texture, vTexCoords) : uMaterial.diffuseColor;
        
        objectColor = texColor.rgb;
        alpha = texColor.a;
    }
    
    if (uMaterial.hasDetailTexture)
    {
        vec3 detailColor = texture(uMaterial.detailTexture, vTexCoords * 128.0).rgb;
        objectColor *= detailColor * 2.0;
    }

    vec3 result;

    if (uIsWater || uMaterial.lighting)
    {
        // --- Ambient ---
        vec3 ambient = ((uLighting.ambient + uLighting.globalAmbient) * 2.0) * objectColor;
        
        // --- Diffuse ---
        vec3 norm = normalize(vNormal);
        vec3 lightDir = normalize(uSunLightDir);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * uLighting.diffuse * objectColor;
        
        // --- Specular ---
        vec3 specular = vec3(0.0);
        if (uMaterial.lightingSpecular)
        {
            vec3 viewDir = normalize(uViewPos - vFragPos);
            vec3 reflectDir = reflect(lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.specularPower);
            specular = spec * uLighting.specular * uMaterial.specularColor; 
        }

        result = ambient + diffuse + specular;
    } else {
        result = objectColor; 
    }

    if (uFog.enabled && !uIsSky)
    {
        float distance = length(uViewPos - vFragPos);
        float fogFactor = clamp((uFog.end - distance) / (uFog.end - uFog.start), 0.0, 1.0);
        result = mix(uFog.color, result, fogFactor);
    }

    FragColor = vec4(result, alpha);
}