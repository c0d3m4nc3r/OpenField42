#pragma once

struct RenderContext
{
    std::vector<glm::mat4> transforms;
    bool wireframe_enabled = false;
};
