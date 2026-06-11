#pragma once

#include "render/camera.h"
#include "core/config.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

#include <string>

struct EngineStats
{
    float fps, delta_time;
    float update_time_ms;
    float render_time_ms;
    float total_frame_time_ms;
};

class Geometry;
class Shader;
class Game
{
public:

    float view_distance = 400.0f;

    bool init();
    void shutdown();
    void tick(float dt, float fps, Uint64 frequency);
    
    void onEvent(const SDL_Event& event);

    bool loadLevel(const std::string& name);

    float getFPS() const { return _stats.fps; }
    float getDeltaTime() const { return _stats.delta_time; }

    bool isRunning() const { return _running; }

private:

    Camera _camera;
    float _camera_speed = CAMERA_MOVE_SPEED;

    EngineStats _stats;

    bool _fullscreen = false;
    bool _draw_debug_info = false;
    bool _running = false;

    void update();
    void render();
    
    bool loadGameObjs();
};

extern Game g_Game;