#pragma once

#include "render/camera.h"
#include "core/config.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

#include <string>

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

    float getFPS() const { return _fps; }
    float getDeltaTime() const { return _delta_time; }

    bool isRunning() const { return _running; }

private:

    Camera _camera;
    float _camera_speed = CAMERA_MOVE_SPEED;

    float _fps;
    float _delta_time;
    float _update_time_ms;
    float _render_time_ms;
    float _total_frame_time_ms;

    bool _fullscreen = false;
    bool _draw_debug_info = false;
    bool _running = false;

    void update();
    void render();
    
    bool loadGameObjs();
};

extern Game g_Game;