#pragma once

#include "render/render_pass.h"

#define DEFINE_RENDER_PASS(ClassName, PassType) \
class ClassName : public RenderPass          \
{                                               \
public:                                         \
    using RenderPass::RenderPass;               \
    void execute(RenderContext& ctx) override;  \
    Type getType() override { return Type::PassType; } \
};

DEFINE_RENDER_PASS(StandardOpaquePass, Standard_Opaque)
DEFINE_RENDER_PASS(StandardTransparentPass, Standard_Transparent)
DEFINE_RENDER_PASS(TreeOpaquePass, Tree_Opaque)
DEFINE_RENDER_PASS(TreeTransparentPass, Tree_Transparent)
DEFINE_RENDER_PASS(SkyPass, Water)
DEFINE_RENDER_PASS(TerrainPass, Water)
DEFINE_RENDER_PASS(WaterPass, Water)

#undef DEFINE_RENDER_PASS