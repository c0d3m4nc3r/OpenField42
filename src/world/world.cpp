#include "world/world.h"

#include "core/console.h"
#include "core/globals.h"
#include "object/object.h"
#include "render/renderer.h"
#include "world/sky.h"
#include "world/terrain.h"
#include "world/water.h"

void World::registerCmds() const
{
    // Sky

    g_Console->registerCmd("Sky.initSky", [](Console::ExecContext& ctx, const Console::CommandArgs& args) -> CommandResult
    {
        bool success = g_World->getSky().init(ctx.last_geom_tmpl);
        if (!success)
        {
            return CommandResult{ "Failed to initialize sky!", CommandStatus::Error };
        }
        return CommandResult{};
    });

    g_Console->bindProperty("Sky.rotAngle", &g_World->getSky(), &Sky::rot_angle);
    g_Console->addAlias("Sky.setRotAngle", "Sky.rotAngle");
    g_Console->addAlias("Sky.sunLightDirectionVec", "Renderer.sunDirection");

    // Water

    g_Console->bindProperty("Water.texLayer1", []() { return std::string(); }, [](const std::string& value) {
        g_World->getWater().setTexture(0, g_TextureMgr->load(value));
    });

    g_Console->bindProperty("Water.texLayer2", []() { return std::string(); }, [](const std::string& value) {
        g_World->getWater().setTexture(1, g_TextureMgr->load(value));
    });

    g_Console->bindProperty("Water.scrollDirection1",
        []() { return g_World->getWater().getLayer(0).scroll_dir; },
        [](const glm::vec2& value) { g_World->getWater().setScrollDir(0, value); }
    );

    g_Console->bindProperty("Water.scrollDirection2",
        []() { return g_World->getWater().getLayer(1).scroll_dir; },
        [](const glm::vec2& value) { g_World->getWater().setScrollDir(1, value); }
    );

    g_Console->bindProperty("Water.scrollLayer1",
        []() { return g_World->getWater().getLayer(0).scroll_speed; },
        [](float value) { g_World->getWater().setScrollSpeed(0, value); }
    );

    g_Console->bindProperty("Water.scrollLayer2",
        []() { return g_World->getWater().getLayer(1).scroll_speed; },
        [](float value) { g_World->getWater().setScrollSpeed(1, value); }
    );

    g_Console->bindProperty("Water.tileLayer1",
        []() { return g_World->getWater().getLayer(0).uv_scale; },
        [](float value) { g_World->getWater().setUVScale(0, value); }
    );

    g_Console->bindProperty("Water.tileLayer2",
        []() { return g_World->getWater().getLayer(1).uv_scale; },
        [](float value) { g_World->getWater().setUVScale(1, value); }
    );

    g_Console->bindProperty("Water.color", &g_World->getWater(), &Water::getColor, &Water::setColor);
    g_Console->bindProperty("Water.deepColor", &g_World->getWater(), &Water::getDeepColor, &Water::setDeepColor);
    g_Console->bindProperty("Water.waterColorDepth", &g_World->getWater(), &Water::getColorDepth, &Water::setColorDepth);
    g_Console->bindProperty("Water.waterAlphaDepth", &g_World->getWater(), &Water::getAlphaDepth, &Water::setAlphaDepth);
    g_Console->bindProperty("Water.waterShallowAlpha", &g_World->getWater(), &Water::getShallowAlpha, &Water::setShallowAlpha);
}

void World::clear()
{
    LOG_INFO("World::clear: Clearing world...");

    _water.clear();
    _sky.clear();
    _terrain.clear();

    LOG_INFO("World::clear: World cleared!");
}

void World::render()
{
    auto* terrain_geom = _terrain.getGeometry();
    if (terrain_geom) g_Renderer->submit(terrain_geom, glm::mat4(1.0f));

    auto* sky_geom = _sky.getGeometry();
    if (sky_geom)
    {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::rotate(transform, glm::radians(_sky.rot_angle), glm::vec3(0.0f, 1.0f, 0.0f));
        g_Renderer->submit(sky_geom, transform);
    }

    auto* water_geom = _water.getGeometry();
    if (water_geom) g_Renderer->submit(water_geom, glm::mat4(1.0f));
}
