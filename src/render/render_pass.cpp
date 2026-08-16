#include "render_pass.h"

void RenderPass::add(const RenderCommand& cmd)
{
    if (!cmd.vao || !cmd.index_count) return;
    queue.push_back(cmd);
    
    _stats.meshes_rendered++;
    _stats.polygons_rendered += cmd.index_count / 3;
}
