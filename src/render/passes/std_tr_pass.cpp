#include "render/render_passes.h"
#include "render/render_context.h"

#include "geometry/material.h"
#include "render/shader.h"

#include "glad/gl.h"

#include <algorithm>

void StandardTransparentPass::execute(RenderContext& ctx)
{
    if (queue.empty() || !shader) return;

    shader->use();

    std::sort(queue.begin(), queue.end(), 
        [](const RenderCommand& a, const RenderCommand& b) {
            return a.distance_to_camera > b.distance_to_camera;
        });

    uint32_t last_vao = 0;
    uint32_t last_transform_id = -1;
    
    for (auto& cmd : queue)
    {   
        if (cmd.material)
        {
            cmd.material->apply(shader);
            if (cmd.material->no_depth_write)
                glDepthMask(GL_FALSE);
        }
        
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

        glDepthMask(GL_TRUE);
    }

    glBindVertexArray(0);

    queue.clear();
}
