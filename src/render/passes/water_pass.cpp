#include "core/globals.h"
#include "render/render_passes.h"

#include "render/render_context.h"
#include "render/shader.h"
#include "render/texture.h"

#include "glad/gl.h"
#include "world/world.h"

void WaterPass::onExecute(RenderContext& ctx)
{
    Shader* shader = getShader();

    shader->use();
    
    shader->setFloat("u_Time", (float)SDL_GetTicks() / 1000.0f);
    shader->setBool("u_WireframeEnabled", ctx.wireframe_enabled);

    glDisable(GL_CULL_FACE);

    unsigned int last_vao = 0;
    unsigned int last_transform_id = 0;
    TextureHandle last_tex0;
    TextureHandle last_tex1;
    TextureHandle last_tex2;

    unsigned int halfvec_lut = g_World->getWater().getHalfVecLUT();
    if (halfvec_lut != 0)
    {
        glBindTextureUnit(3, halfvec_lut);
        shader->setInt("u_HalfVecLUT", 3);
    }

    for (const auto& cmd : queue)
    {
        if (cmd.textures[0].id != last_tex0.id && cmd.textures[0].isValid())
        {
            g_TextureMgr->get(cmd.textures[0]).bind(0);
            shader->setInt("u_TexLayer1", 0);
            last_tex0 = cmd.textures[0];
        }

        if (cmd.textures[1].id != last_tex1.id && cmd.textures[1].isValid())
        {
            g_TextureMgr->get(cmd.textures[1]).bind(1);
            shader->setInt("u_TexLayer2", 1);
            last_tex1 = cmd.textures[1];
        }

        if (cmd.textures[2].id != last_tex1.id && cmd.textures[2].isValid())
        {
            g_TextureMgr->get(cmd.textures[2]).bind(2);
            shader->setInt("u_TexNormal", 2);
            last_tex2 = cmd.textures[2];
        }

        if (cmd.transform_id != last_transform_id)
        {
            shader->setMat4("u_Model", ctx.transforms[cmd.transform_id]);
            last_transform_id = cmd.transform_id;
        }

        if (cmd.vao != last_vao) {
            glBindVertexArray(cmd.vao);
            last_vao = cmd.vao;
        }

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            cmd.index_count,
            GL_UNSIGNED_INT,
            cmd.index_offset,
            cmd.base_vertex
        );
    }

    glEnable(GL_CULL_FACE);

    glBindVertexArray(0);
}
