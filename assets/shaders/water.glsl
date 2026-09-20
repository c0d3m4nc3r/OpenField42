#include "common/fog.glsl"
#include "common/lighting.glsl"

layout (std140, binding = 0) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_ViewPos;
};

layout(std140, binding = 3) uniform WaterBlock
{
    // xy = dir, z = speed, w = uv scale
    vec4 u_Layer1;
    vec4 u_Layer2;
    vec4 u_LayerNormalMap;
    vec4 u_SpecularColor;
};

#ifdef VERTEX // ---

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 4) in vec4 a_Color;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec4 v_Color;
out vec3 v_HalfVecLUT_Dir;

uniform mat4 u_Model;

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    
    v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    v_Normal = normalize(mat3(transpose(inverse(u_Model))) * a_Normal);
    
    vec4 ecPos = u_View * vec4(v_FragPos, 1.0);
    ComputeFogCoord(ecPos.xyz);
    gl_Position = u_Projection * ecPos;

    vec3 viewDir = normalize(v_FragPos - u_ViewPos.rgb);
    v_HalfVecLUT_Dir = reflect(viewDir, v_Normal);
    v_HalfVecLUT_Dir.xz = -v_HalfVecLUT_Dir.xz;
}

#endif // VERTEX

#ifdef FRAGMENT // ---

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec4 v_Color;
in vec3 v_HalfVecLUT_Dir;

out vec4 f_Color;

uniform sampler2D u_TexLayer1;
uniform sampler2D u_TexLayer2;
uniform sampler2D u_TexNormal;
uniform samplerCube u_HalfVecLUT;

uniform float u_Time;
uniform bool u_WireframeEnabled;

void main()
{
    if (u_WireframeEnabled)
    {
        f_Color = vec4(0.0, 0.0, 1.0, 1.0);
        return;
    }

    vec2 uv1 = (v_TexCoord * u_Layer1.w) + u_Layer1.xy * u_Layer1.z * u_Time;
    vec2 uv2 = (v_TexCoord * u_Layer2.w) + u_Layer2.xy * u_Layer2.z * u_Time;
    vec2 uvNormal = (v_TexCoord * u_LayerNormalMap.w) + u_LayerNormalMap.xy * u_LayerNormalMap.z * u_Time;

    vec4 layerColor1 = texture(u_TexLayer1, uv1);
    vec4 layerColor2 = texture(u_TexLayer2, uv2);
    vec3 finalColor = (layerColor1 * layerColor2).rgb * v_Color.rgb;

    vec3 normal = texture(u_TexNormal, uvNormal).rgb;
    vec3 half_vec = texture(u_HalfVecLUT, v_HalfVecLUT_Dir).rgb;

    float spec = clamp(dot(normal - 0.5, half_vec - 0.5) * 4.0, 0.0, 1.0);

    finalColor += spec * u_SpecularColor.rgb;
    
    ApplyFog(finalColor);

    f_Color = vec4(finalColor, v_Color.a);
}

#endif // FRAGMENT
