#include "common/fog.glsl"

layout (std140, binding = 0) uniform CameraBlock
{
    mat4 u_View;
    mat4 u_Projection;
    vec4 u_ViewPos;
};

#ifdef VERTEX // ---

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec2 a_TexCoord2;

out vec2 v_TexCoord;
out vec2 v_TexCoord2;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 u_Model;

void main()
{
    v_TexCoord = a_TexCoord;
    v_TexCoord2 = a_TexCoord2;
    v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;

    vec4 ecPos = u_View * vec4(v_FragPos, 1.0);
    ComputeFogCoord(ecPos.xyz);
    gl_Position = u_Projection * ecPos;
}

#endif // VERTEX

#ifdef FRAGMENT // ---

in vec2 v_TexCoord;
in vec2 v_TexCoord2;
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

    vec3 baseColor = texture(u_BaseTex, v_TexCoord).rgb;
    vec3 detailColor = texture(u_DetailTex, v_TexCoord2).rgb;
    vec3 result = clamp(detailColor + (baseColor - vec3(0.5)), 0.0, 1.0);

    ApplyFog(result);

    f_Color = vec4(result, 1.0);
}

#endif // FRAGMENT