#include "core/globals.h"
#include "render/render_passes.h"

#include "render/render_context.h"
#include "render/shader.h"
#include "render/texture.h"

#include "glad/gl.h"

void TerrainPass::execute(RenderContext& ctx)
{
    Shader* shader = getShader();
    
    if (queue.empty() || !shader) return;
    
    shader->use();

    shader->setBool("u_WireframeEnabled", ctx.wireframe_enabled);

    uint32_t last_vao = 0;
    uint32_t last_transform_id = -1;

    for (auto& cmd : queue)
    {
        g_TextureMgr->get(cmd.textures[0]).bind(0);
        shader->setInt("u_BaseTex", 0);
        
        g_TextureMgr->get(cmd.textures[1]).bind(1);
        shader->setInt("u_DetailTex", 1);

        if (last_transform_id != cmd.transform_id) {
            shader->setMat4("u_Model", ctx.transforms[cmd.transform_id]);
            last_transform_id = cmd.transform_id;
        }

        if (last_vao != cmd.vao) {
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

    glBindVertexArray(0);

    queue.clear();
}
