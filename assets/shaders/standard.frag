#version 330 core

layout (std140) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_ViewPos;
};

layout (std140) uniform FogBlock
{
    vec4 u_FogColor;
    vec4 u_FogParams; // x - start, y - end, z - enabled, w - padding
};

layout (std140) uniform LightingBlock
{
    vec4 u_DiffuseLight;
    vec4 u_SpecularLight;
    vec4 u_AmbientLight;
    vec4 u_GlobalAmbientLight;
    vec4 u_SunDirection;
};

in vec2 v_TexCoords;
in vec3 v_Normal;
in vec3 v_FragPos;

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
        f_Color = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 objectColor = u_Material.diffuseColor.rgb;
    float alpha = u_Material.diffuseColor.a;

    if (u_Material.hasTexture)
    {
        vec4 texColor = texture(u_Material.texture, v_TexCoords);
        objectColor = texColor.rgb;
        alpha = texColor.a;
    }

    if (u_Material.billboard)
    {
        objectColor.rgb *= 2.0;
    }
    
    if (u_Material.hasDetailTexture)
    {
        vec3 detailColor = texture(u_Material.detailTexture, v_TexCoords * 128.0).rgb;
        objectColor *= detailColor * 2.0;
    }

    vec3 result;
    if (u_Material.lighting)
    {
        vec3 ambient = ((u_AmbientLight.rgb + u_GlobalAmbientLight.rgb) * 2.0) * objectColor;

        vec3 norm = normalize(v_Normal);
        vec3 lightDir = normalize(u_SunDirection.rgb);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * u_DiffuseLight.rgb * objectColor;

        vec3 specular = vec3(0.0);
        if (u_Material.lightingSpecular)
        {
            vec3 viewDir = normalize(u_ViewPos.xyz - v_FragPos);
            vec3 reflectDir = reflect(lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.specularPower);
            specular = spec * u_SpecularLight.rgb * u_Material.specularColor;
        }

        result = ambient + diffuse + specular;
    } 
    else 
    {
        result = objectColor;
    }

    if (u_FogParams.z == 1.0)
    {
        float distance = length(u_ViewPos.xyz - v_FragPos);
        float fogFactor = clamp((u_FogParams.y - distance) / (u_FogParams.y - u_FogParams.x), 0.0, 1.0);
        result = mix(u_FogColor.rgb, result, fogFactor);
    }

    f_Color = vec4(result, alpha);
}
