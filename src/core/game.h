#pragma once

#include "render/camera.h"
#include "core/config.h"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>

#include <string>
#include <memory>

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

    float getFPS() const;
    float getDeltaTime() const;

    bool isRunning() const;

private:

    Camera camera;
    float camera_speed = CAMERA_MOVE_SPEED;

    std::shared_ptr<Shader> main_shader;

    float fps;
    float delta_time;
    float update_time_ms;
    float render_time_ms;
    float total_frame_time_ms;

    bool fullscreen = false;
    bool draw_debug_info = false;
    bool is_running = false;

    void update();
    void render();
    
    bool loadGameObjs();
    bool loadResources();
};

extern Game game;