#pragma once

#include "render/render_command.h"

class Shader;
struct RenderContext;

class RenderPass
{
public:

    enum class Type : unsigned char
    {
        Opaque, Transparent, Sky, Water, Terrain, Count
    };

    RenderPass(Shader* shader) : shader(shader) {}
    virtual ~RenderPass() = default;
    
    void add(const RenderCommand& cmd);

    virtual void execute(RenderContext& ctx) = 0;

    virtual Type getType() = 0;

    void setShader(Shader* new_shader) { shader = new_shader; }

protected:
    
    Shader* shader = nullptr;
    std::vector<RenderCommand> queue;

};