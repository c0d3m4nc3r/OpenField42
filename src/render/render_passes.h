#pragma once

#include "render/render_pass.h"

#define DEFINE_RENDER_PASS(PassType) \
class PassType##Pass : public RenderPass          \
{                                               \
public:                                         \
    using RenderPass::RenderPass;               \
    void execute(RenderContext& ctx) override;  \
    Type getType() override { return Type::PassType; } \
};

DEFINE_RENDER_PASS(Opaque)
DEFINE_RENDER_PASS(Transparent)
DEFINE_RENDER_PASS(Sky)
DEFINE_RENDER_PASS(Water)
DEFINE_RENDER_PASS(Terrain)

#undef DEFINE_RENDER_PASS