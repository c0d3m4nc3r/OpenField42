#include "render/render_passes.h"
#include "render/render_context.h"

#include "geometry/material.h"
#include "render/shader.h"

#include "glad/gl.h"

void SkyPass::execute(RenderContext& ctx)
{
    Shader* shader = getShader();

    if (queue.empty() || !shader) return;

    shader->use();

    shader->setBool("u_WireframeEnabled", ctx.wireframe_enabled);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    
    uint32_t last_vao = 0;
    uint32_t last_transform_id = -1;

    for (auto& cmd : queue)
    {
        if (cmd.material)
            cmd.material->apply(shader);
    
        if (last_vao != cmd.vao) {
            glBindVertexArray(cmd.vao);
            last_vao = cmd.vao;
        }

        if (last_transform_id != cmd.transform_id) {
            shader->setMat4("u_Model", ctx.transforms[cmd.transform_id]);
            last_transform_id = cmd.transform_id;
        }

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            cmd.index_count,
            GL_UNSIGNED_INT,
            cmd.index_offset,
            cmd.base_vertex
        );
    }

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    
    glBindVertexArray(0);

    queue.clear();
}