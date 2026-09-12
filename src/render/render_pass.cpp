#include "render_pass.h"

bool RenderPass::add(const RenderCommand& cmd)
{
    if (!_enabled || !cmd.vao || !cmd.index_count) return false;
    
    queue.push_back(cmd);
    
    _stats.meshes_rendered++;
    _stats.polygons_rendered += cmd.index_count / 3;

    return true;
}

void RenderPass::execute(RenderContext& ctx)
{
    if (!_enabled || queue.empty() || !_shader) return;
    onExecute(ctx);
    queue.clear();
}