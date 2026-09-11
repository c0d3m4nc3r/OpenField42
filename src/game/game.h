#pragma once

#include "render/camera.h"
#include "core/config.h"

#include <SDL3/SDL_events.h>

#include <string>

class Game
{
public:

    bool init();
    void registerCmds() const;
    
    void update(float dt);

    void onEvent(const SDL_Event& event);

    bool loadLevel(const std::string& name);    
    
    void teleport(const glm::vec3& position);

    float getViewDistance() const;
    void setViewDistance(float distance);


private:

    Camera _camera;
    float _camera_speed = CAMERA_MOVE_SPEED;

    bool _fullscreen = false;
    bool _objs_loaded = false;
    
    bool loadGameObjs();
};
