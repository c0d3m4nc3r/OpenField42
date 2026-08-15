#include "core/globals.h"
#include "render/render_passes.h"

#include "render/render_context.h"
#include "render/shader.h"
#include "render/texture.h"

#include "glad/gl.h"

void WaterPass::execute(RenderContext& ctx)
{
    if (queue.empty() || !shader) return;

    shader->use();
    
    shader->setFloat("u_Time", (float)SDL_GetTicks() / 1000.0f);
    shader->setBool("u_WireframeEnabled", ctx.wireframe_enabled);

    glDisable(GL_CULL_FACE);

    unsigned int last_vao = 0;
    unsigned int last_transform_id = 0;
    TextureHandle last_tex1;
    TextureHandle last_tex2;

    for (const auto& cmd : queue)
    {
        if (cmd.textures[0].id != last_tex1.id && cmd.textures[0].isValid())
        {
            g_TextureMgr->get(cmd.textures[0]).bind(0);
            shader->setInt("u_TexLayer1", 0);
            last_tex1 = cmd.textures[0];
        }

        if (cmd.textures[1].id != last_tex2.id && cmd.textures[1].isValid())
        {
            g_TextureMgr->get(cmd.textures[1]).bind(1);
            shader->setInt("u_TexLayer2", 1);
            last_tex2 = cmd.textures[1];
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
    
    queue.clear();
}
