#include "core/engine.h"

#include "core/console.h"
#include "core/debugui.h"
#include "core/globals.h"
#include "game/game.h"
#include "geometry/geometry_manager.h"
#include "platform/input.h"
#include "platform/window.h"
#include "render/renderer.h"
#include "render/shader_manager.h"
#include "render/texture_manager.h"
#include "vfs/providers.h"
#include "vfs/vfs.h"
#include "world/terrain.h"
#include "world/water.h"
#include "world/world.h"

Game* g_Game = nullptr;
Window* g_Window = nullptr;
Input* g_Input = nullptr;
Renderer* g_Renderer = nullptr;
Console* g_Console = nullptr;
VFS* g_VFS = nullptr;
DebugUI* g_DebugUI = nullptr;
GeometryManager* g_GeometryMgr = nullptr;
ShaderManager* g_ShaderMgr = nullptr;
TextureManager* g_TextureMgr = nullptr;
World* g_World = nullptr;

std::string SHADERS_TO_LOAD[] = {"sky", "standard", "terrain", "water"};

bool Engine::init(int argc, char* argv[])
{
    LOG_INFO("Engine::init: Initializing engine...");

    g_Game = new Game();
    g_Window = new Window();
    g_Input = new Input();
    g_Renderer = new Renderer();
    g_Console = new Console();
    g_VFS = new VFS();
    g_DebugUI = new DebugUI();
    g_GeometryMgr = new GeometryManager();
    g_ShaderMgr = new ShaderManager();
    g_TextureMgr = new TextureManager();
    g_World = new World();

    if (!g_Window->init())
    {
        LOG_ERROR("Engine::init: Failed to initialize window!");
        return false;
    }

    g_VFS->mountProvider(std::make_shared<FolderProvider>("assets"));

    for (const auto& name : SHADERS_TO_LOAD)
    {
        if (!g_ShaderMgr->load(name, "shaders/" + name + ".glsl"))
        {
            LOG_ERROR("Engine::init: Failed to load shaders!");
            return false;
        }
    }

    if (!g_Renderer->init())
    {
        LOG_ERROR("Engine::init: Failed to initialize renderer!");
        return false;
    }

    g_DebugUI->init();
    g_Console->init();
    g_Game->init();

    std::string level_name = "Market_Garden";
    if (argc > 1) level_name = argv[1];
    
    if (!g_Game->loadLevel(level_name))
    {
        LOG_ERROR("Engine::init: Failed to load level '%s'!", level_name.c_str());
        return 3;
    }

    LOG_INFO("Engine::init: Engine initialized!");

    _is_running = true;

    return true;
}

void Engine::shutdown()
{
    LOG_INFO("Engine::shutdown: Shutting down engine...");

    g_World->getWater().shutdown();
    g_World->getTerrain().shutdown();
    g_Renderer->shutdown();
    g_ShaderMgr->unloadAll();
    g_TextureMgr->clear();
    g_Window->shutdown();
    g_VFS->unmountAll();

    delete g_World; g_World = nullptr;
    delete g_TextureMgr; g_TextureMgr = nullptr;
    delete g_ShaderMgr; g_ShaderMgr = nullptr;
    delete g_GeometryMgr; g_GeometryMgr = nullptr;
    delete g_Console; g_Console = nullptr;
    delete g_DebugUI; g_DebugUI = nullptr;
    delete g_Renderer; g_Renderer = nullptr;
    delete g_Input; g_Input = nullptr;
    delete g_Window; g_Window = nullptr;

    LOG_INFO("Engine::shutdown: Engine shutdown!");
}

void Engine::tick(float dt, float fps, uint64_t frequency)
{
    _stats.delta_time = dt;
    _stats.fps = fps;

    Uint64 frame_start = SDL_GetPerformanceCounter();
    
    Uint64 update_start = SDL_GetPerformanceCounter();
    update(dt);
    Uint64 update_end = SDL_GetPerformanceCounter();
    _stats.update_time_ms = (float)(update_end - update_start) / frequency * 1000.0f;
    
    Uint64 render_start = SDL_GetPerformanceCounter();
    render();
    Uint64 render_end = SDL_GetPerformanceCounter();
    _stats.render_time_ms = (float)(render_end - render_start) / frequency * 1000.0f;
    
    Uint64 frame_end = SDL_GetPerformanceCounter();
    _stats.total_frame_time_ms = (float)(frame_end - frame_start) / frequency * 1000.0f;
}

void Engine::update(float dt)
{
    g_Input->update();
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
            {
                _is_running = false;
                break;
            }
        }

        g_DebugUI->onEvent(event);
        g_Input->onEvent(event);
        g_Game->onEvent(event);
    }

    g_World->update(dt);
    g_Game->update(dt);

    auto& water = g_World->getWater();

    if (water.isDirty())
    {
        auto& layer1 = water.getLayer(0);
        auto& layer2 = water.getLayer(1);

        Renderer::WaterParams water_params;
        water_params.layer_1 = glm::vec4(layer1.scroll_dir, layer1.scroll_speed, layer1.uv_scale);
        water_params.layer_2 = glm::vec4(layer2.scroll_dir, layer2.scroll_speed, layer2.uv_scale);
        water_params.tex_layer1 = layer1.texture;
        water_params.tex_layer2 = layer2.texture;
        g_Renderer->setWaterParams(water_params);
        water.clearDirty();
    }

    auto& terrain = g_World->getTerrain();

    g_Renderer->setTerrainTextures(
        terrain.getBaseTexture(),
        terrain.getDetailTexture()
    );
}

void Engine::render()
{
    g_TextureMgr->updateGpuUploads(4);
    g_Renderer->resetStats();
    g_World->render();
    g_Renderer->flush();
    g_DebugUI->render(_stats);
    g_Window->swapBuffers();
}
