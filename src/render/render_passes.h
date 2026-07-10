#pragma once

#include "render/render_pass.h"

class OpaquePass : public RenderPass
{
public:

    using RenderPass::RenderPass;

    void execute(RenderContext& ctx) override;

    Type getType() override { return Type::Opaque; }
};

class TransparentPass : public RenderPass
{
public:

    using RenderPass::RenderPass;

    void execute(RenderContext& ctx) override;

    Type getType() override { return Type::Transparent; }
};

class SkyPass : public RenderPass
{
public:

    using RenderPass::RenderPass;

    void execute(RenderContext& ctx) override;

    Type getType() override { return Type::Sky; }
};

class WaterPass : public RenderPass
{
public:

    using RenderPass::RenderPass;

    void execute(RenderContext& ctx) override;

    Type getType() override { return Type::Water; }
};


