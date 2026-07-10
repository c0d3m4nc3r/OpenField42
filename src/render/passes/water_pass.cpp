#include "render/render_passes.h"

#include "render/render_context.h"
#include "render/shader.h"
#include "render/texture.h"

#include "glad/gl.h"

void WaterPass::execute(RenderContext& ctx)
{
    if (queue.empty()) return;

    shader.use();
    
    shader.setFloat("uTime", (float)SDL_GetTicks() / 1000.0f);
    shader.setBool("uWireframeEnabled", ctx.wireframe_enabled);

    glDisable(GL_CULL_FACE);

    unsigned int last_vao = 0;
    unsigned int last_transform_id = 0;
    Texture* last_tex1 = nullptr;
    Texture* last_tex2 = nullptr;

    for (const auto& cmd : queue)
    {
        if (cmd.textures[0] != last_tex1 && cmd.textures[0] != nullptr)
        {
            cmd.textures[0]->bind(0);
            shader.setInt("uTexLayer1", 0);
            last_tex1 = cmd.textures[0];
        }

        if (cmd.textures[1] != last_tex2 && cmd.textures[1] != nullptr)
        {
            cmd.textures[1]->bind(1);
            shader.setInt("uTexLayer2", 1);
            last_tex2 = cmd.textures[1];
        }

        if (cmd.transform_id != last_transform_id)
        {
            shader.setMat4("uModel", ctx.transforms[cmd.transform_id]);
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
