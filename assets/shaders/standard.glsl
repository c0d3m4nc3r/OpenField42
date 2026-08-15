#include "common/material.glsl"
#include "common/lighting.glsl"
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
layout(location = 4) in vec2 a_SpriteOfs;

out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 u_Model;

void main()
{
    v_TexCoords = a_TexCoord;

    if (IsBillboard()) 
    {
        vec3 worldPos = vec3(u_Model * vec4(a_Pos, 1.0));

        vec3 camRight = vec3(u_View[0][0], u_View[1][0], u_View[2][0]);
        vec3 camUp    = vec3(u_View[0][1], u_View[1][1], u_View[2][1]);

        worldPos += camRight * a_SpriteOfs.x;
        worldPos += camUp    * a_SpriteOfs.y;

        v_FragPos = worldPos;
        v_Normal = -vec3(u_View[0][2], u_View[1][2], u_View[2][2]);
    }
    else 
    {
        v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
        v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    }

    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}

#endif // VERTEX

#ifdef FRAGMENT // ---

in vec2 v_TexCoords;
in vec3 v_Normal;
in vec3 v_FragPos;

out vec4 f_Color;

uniform bool u_WireframeEnabled;

void main()
{
    if (u_WireframeEnabled)
    {
        f_Color = vec4(1.0, 0.0, 0.0, 1.0);
        return;
    }

    float alpha = u_Material.diffuse.a;

    vec4 texColor = texture(u_MainTexture, v_TexCoords);
    vec3 objectColor = texColor.rgb * u_Material.diffuse.rgb;
    alpha = texColor.a;
    
    if (IsBillboard())
    {
        objectColor.rgb *= 2.0;
    }
    
    if (HasDetailTexture())
    {
        vec3 detailColor = texture(u_DetailTexture, v_TexCoords * 128.0).rgb;
        objectColor *= detailColor * 2.0;
    }

    vec3 result;
    if (IsLightingEnabled()) {
        result = CalcDirLight(v_FragPos, v_Normal, u_ViewPos.xyz, objectColor);
    } else {
        result = objectColor;
    }

    ApplyFog(result, length(u_ViewPos.xyz - v_FragPos));

    f_Color = vec4(result, alpha);
}

#endif // FRAGMENT