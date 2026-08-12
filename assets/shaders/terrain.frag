#version 450 core

layout (std140, binding = 0) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_ViewPos;
};

layout (std140, binding = 1) uniform FogBlock
{
    vec4 u_FogColor;
    vec4 u_FogParams; // x - start, y - end, z - enabled, w - padding
};

in vec2 v_TexCoords;
in vec3 v_Normal;
in vec3 v_FragPos;

out vec4 f_Color;

uniform sampler2D u_BaseTex;
uniform sampler2D u_DetailTex;

uniform bool u_WireframeEnabled;

void main()
{
    if (u_WireframeEnabled)
    {
        f_Color = vec4(0.0, 1.0, 0.0, 1.0);
        return;
    }

    vec3 baseColor = texture(u_BaseTex, v_TexCoords).rgb;
    vec3 detailColor = texture(u_DetailTex, v_TexCoords * 128).rgb;
    vec3 result = baseColor * detailColor * 2.0;

    if (u_FogParams.z == 1.0)
    {
        float distance = length(u_ViewPos.xyz - v_FragPos);
        float fogFactor = clamp((u_FogParams.y - distance) / (u_FogParams.y - u_FogParams.x), 0.0, 1.0);
        result = mix(u_FogColor.rgb, result, fogFactor);
    }

    f_Color = vec4(result, 1.0);
}
