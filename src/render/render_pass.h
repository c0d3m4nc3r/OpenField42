#pragma once

#include "render/render_command.h"

class Shader;
struct RenderContext;

class RenderPass
{
public:

    enum class Type : unsigned char
    {
        Standard_Opaque, Standard_Transparent,
        Tree_Opaque, Tree_Transparent,
        Sky, Terrain, Water,
        Count
    };

    struct Stats
    {
        size_t meshes_rendered = 0;
        size_t polygons_rendered = 0;
    };

    RenderPass(Shader* shader) : _shader(shader) {}
    virtual ~RenderPass() = default;
    
    void add(const RenderCommand& cmd);

    void clearStats() { _stats = {0, 0}; }

    virtual void execute(RenderContext& ctx) = 0;

    virtual Type getType() = 0;

    Shader* getShader() const { return _shader; }
    const Stats& getStats() const { return _stats; }

protected:
    
    std::vector<RenderCommand> queue;

private:

    Shader* _shader = nullptr;
    Stats _stats;

};

inline std::string passTypeToString(RenderPass::Type type)
{
    switch (type)
    {
    case RenderPass::Type::Standard_Opaque: return "Standard Opaque";
    case RenderPass::Type::Standard_Transparent: return "Standard Transparent";
    case RenderPass::Type::Tree_Opaque: return "Tree Opaque";
    case RenderPass::Type::Tree_Transparent: return "Tree Transparent";
    case RenderPass::Type::Sky: return "Sky";
    case RenderPass::Type::Water: return "Water";
    case RenderPass::Type::Terrain: return "Terrain";
    default: return "Unknown";
    }
}