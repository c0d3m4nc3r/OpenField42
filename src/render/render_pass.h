#pragma once

#include "render/render_command.h"

class Shader;
struct RenderContext;

class RenderPass
{
public:

    enum class Type : unsigned char
    {
        Opaque, Transparent, Sky, Water, Count
    };

    RenderPass(Shader& shader) : shader(shader) {}
    virtual ~RenderPass() = default;
    
    void add(const RenderCommand& cmd);

    virtual void execute(RenderContext& ctx) = 0;

    virtual Type getType() = 0;

protected:
    
    Shader& shader;
    std::vector<RenderCommand> queue;

};