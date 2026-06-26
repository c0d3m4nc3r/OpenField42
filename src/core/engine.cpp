#include "core/engine.h"

#include "core/console.h"
#include "core/debugui.h"
#include "game/game.h"
#include "geometry/geometry_manager.h"
#include "platform/input.h"
#include "platform/window.h"
#include "render/renderer.h"
#include "vfs/vfs.h"
#include "world/world.h"

Engine::Engine() {}
Engine::~Engine() = default;

bool Engine::init(int argc, char* argv[])
{
    LOG_INFO("Engine::init: Initializing engine...");

    _window = std::make_unique<Window>();
    if (!_window->init())
    {
        LOG_ERROR("Engine::init: Failed to initialize window!");
        return false;
    }

    _input = std::make_unique<Input>(*_window);

    VFS::mountProvider(std::make_shared<VFS::FolderProvider>("assets"));

    _renderer = std::make_unique<Renderer>();
    if (!_renderer->init())
    {
        LOG_ERROR("Engine::init: Failed to initialize renderer!");
        return false;
    }

    _debug_ui = std::make_unique<DebugUI>();
    _debug_ui->init(*_window);

    _console = std::make_unique<Console>();
    _console->init(*this);
    
    _geometry_mgr = std::make_unique<GeometryManager>();

    _world = std::make_unique<World>(*_geometry_mgr.get());
    _world->init();

    _game = std::make_unique<Game>(*this);
    _game->init();

    std::string level_name = "Market_Garden";
    if (argc > 1) level_name = argv[1];
    
    if (!_game->loadLevel(level_name))
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

    _geometry_mgr.reset();
    _world->shutdown();
    _renderer->shutdown();
    _window->shutdown();
    VFS::unmountAll();

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
    _input->update();
    
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

        _debug_ui->onEvent(event);
        _input->onEvent(event);
        _game->onEvent(event);
    }

    _world->update(dt);
    _game->update(dt);
}

void Engine::render()
{
    _renderer->resetStats();
    _world->renderTerrain(*_renderer);
    _world->renderObjects(*_renderer);
    _renderer->flush(*_world);
    if (_debug_ui_enabled)
        _debug_ui->render(_stats, *_renderer);
    _window->swapBuffers();
}
