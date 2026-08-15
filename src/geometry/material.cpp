#include "geometry/material.h"

#include "core/globals.h"
#include "render/shader.h"
#include "render/texture.h"

#include "glad/gl.h"

#define MAT_FLAG_HAS_DETAIL_TEX   (1 << 0) // 1
#define MAT_FLAG_LIGHTING_ENABLED (1 << 1) // 2
#define MAT_FLAG_SPECULAR_ENABLED (1 << 2) // 4
#define MAT_FLAG_IS_BILLBOARD     (1 << 3) // 8

void Material::apply(Shader* shader) const
{
    if (!shader) return;

    shader->setVec4("u_Material.diffuse", diffuse_color.toVec4());
    shader->setVec4("u_Material.specular", glm::vec4(specular_color.toVec3(), specular_power));

    int flags = 0;

    g_TextureMgr->get(texture).bind(0);
    shader->setInt("u_MainTexture", 0);

    if (detail_texture.isValid())
    {
        g_TextureMgr->get(detail_texture).bind(1);
        shader->setInt("u_DetailTexture", 1);
        flags |= MAT_FLAG_HAS_DETAIL_TEX;
    }

    flags |= (lighting ? MAT_FLAG_LIGHTING_ENABLED : 0);
    flags |= (lighting_specular ? MAT_FLAG_SPECULAR_ENABLED : 0);
    flags |= (billboard ? MAT_FLAG_IS_BILLBOARD : 0);

    shader->setInt("u_Material.flags", flags);

    if (twosided) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
    }
}
