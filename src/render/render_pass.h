#pragma once

#include "render/render_item.h"
#include "utils/string_utils.h"

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
        Count,
        Unknown
    };

    struct Stats
    {
        size_t meshes_rendered = 0;
        size_t polygons_rendered = 0;
    };

    RenderPass(Shader* shader) : _shader(shader) {}
    virtual ~RenderPass() = default;
    
    bool add(const RenderItem& cmd);

    void clearStats() { _stats = {0, 0}; }

    void execute(RenderContext& ctx);

    virtual Type getType() = 0;

    Shader* getShader() const { return _shader; }
    const Stats& getStats() const { return _stats; }

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool enabled) { _enabled = enabled; }

protected:

    virtual void onExecute(RenderContext& ctx) = 0;
    
    std::vector<RenderItem> queue;

private:

    Shader* _shader = nullptr;
    Stats _stats;

    bool _enabled = true;

};

constexpr std::string_view passTypeToString(RenderPass::Type type)
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

inline RenderPass::Type passTypeFromString(std::string_view str)
{   
    static const std::unordered_map<std::string, RenderPass::Type> lut = {
        {"standard_opaque", RenderPass::Type::Standard_Opaque},
        {"standard_transparent", RenderPass::Type::Standard_Transparent},
        {"tree_opaque", RenderPass::Type::Tree_Opaque},
        {"tree_transparent", RenderPass::Type::Tree_Transparent},
        {"sky", RenderPass::Type::Sky},
        {"water", RenderPass::Type::Water},
        {"terrain", RenderPass::Type::Terrain}
    };

    auto it = lut.find(StringUtils::lowercase(str));
    if (it != lut.end())
        return it->second;

    return RenderPass::Type::Unknown;
}