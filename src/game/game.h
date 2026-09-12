#pragma once

#include "render/camera.h"
#include "core/config.h"

#include <SDL3/SDL_events.h>

class Game
{
public:

    bool init();
    void registerCmds();
    
    void update(float dt);

    void onEvent(const SDL_Event& event);

    bool loadLevel(const std::string& name);
    void unloadLevel();
    
    static std::vector<std::string> getLevelsList();

    void teleport(const glm::vec3& position);

    float getViewDistance() const;
    void setViewDistance(float distance);

private:

    Camera _camera;
    float _camera_speed = CAMERA_MOVE_SPEED;

    glm::vec3 _camera_velocity{0.0f};

    bool _fullscreen = false;
    bool _objs_loaded = false;
    bool _cinematic_camera = false;

    std::string _current_level = "";

    glm::vec3 _before_spawn_camera_pos[2];
    
    bool loadGameObjs();
};
