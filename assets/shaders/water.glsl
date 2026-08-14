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

layout(std140, binding = 3) uniform WaterBlock
{
    // xy = dir, z = speed, w = uv scale
    vec4 u_Layer1;
    vec4 u_Layer2;
};

#ifdef VERTEX // ---

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec4 a_Color;

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec4 v_Color;

uniform mat4 u_Model;

void main()
{
    v_Color = a_Color;
    v_TexCoords = a_TexCoord;
    v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Pos, 1.0);
}

#endif // VERTEX

#ifdef FRAGMENT // ---

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec4 v_Color;

out vec4 f_Color;

uniform sampler2D u_TexLayer1;
uniform sampler2D u_TexLayer2;

uniform float u_Time;
uniform bool u_WireframeEnabled;

void main()
{
    if (u_WireframeEnabled)
    {
        f_Color = vec4(0.0, 0.0, 1.0, 1.0);
        return;
    }

    vec2 uv1 = (v_TexCoords * u_Layer1.w) + u_Layer1.xy * u_Layer1.z * u_Time;
    vec2 uv2 = (v_TexCoords * u_Layer2.w) + u_Layer2.xy * u_Layer2.z * u_Time;

    vec4 layerColor1 = texture(u_TexLayer1, uv1);
    vec4 layerColor2 = texture(u_TexLayer2, uv2);

    vec3 finalColor = (layerColor1 * layerColor2).rgb * v_Color.rgb;
    
    if (u_FogParams.z == 1.0)
    {
        float distance = length(u_ViewPos.xyz - v_FragPos);
        float fogFactor = clamp((u_FogParams.y - distance) / (u_FogParams.y - u_FogParams.x), 0.0, 1.0);
        finalColor = mix(u_FogColor.rgb, finalColor, fogFactor);
    }

    f_Color = vec4(finalColor, v_Color.a);
}

#endif // FRAGMENT
