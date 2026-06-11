#pragma once

#include "core/engine.h"
#include "render/camera.h"
#include "core/config.h"

#include <SDL3/SDL_events.h>

#include <string>

class Game
{
public:

    Game(Engine& engine) : _engine(engine) {}
    ~Game() = default;

    bool init();
    void update(float dt);

    void onEvent(const SDL_Event& event);

    bool loadLevel(const std::string& name);    
    
    void setViewDistance(float distance);

private:

    Camera _camera;
    float _camera_speed = CAMERA_MOVE_SPEED;

    bool _fullscreen = false;
    bool _objs_loaded = false;

    Engine& _engine;
    
    bool loadGameObjs();
};
