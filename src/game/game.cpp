#include "game/game.h"

#include "core/console.h"
#include "core/globals.h"
#include "platform/input.h"
#include "platform/window.h"
#include "render/shader_manager.h"
#include "render/renderer.h"
#include "script/script_manager.h"
#include "ui/stats_overlay_ui.h"
#include "ui/ui_manager.h"
#include "utils/log.h"
#include "vfs/providers.h"
#include "vfs/vfs.h"
#include "world/water.h"
#include "world/world.h"

#include <chrono>


bool Game::init()
{
    LOG_INFO("Game::init: Initializing game...");

    g_VFS->mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/standardMesh.rfa"));
    g_VFS->mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/StandardMesh_001.rfa"));
    g_VFS->mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/treeMesh.rfa"));
    g_VFS->mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture.rfa"));
    g_VFS->mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture_001.rfa"));

    g_Input->setMouseCaptured(true);

    const float WORLD_SIZE = 2048.0f;
    glm::vec3 world_center(WORLD_SIZE/2.0f);
    world_center.y = 75.0f;

    _camera.setPosition(world_center);

    g_Renderer->setCamera(&_camera);

    LOG_INFO("Game::init: Game initialized!");

    return true;
}

void Game::registerCmds() const
{
    g_Console->registerCmd("teleport", [](Console::ExecContext& ctx, const Console::CommandArgs& args) -> CommandResult
    {
        if (args.size() < 1)
        {
            return { "Not enough arguments! Usage: teleport|tp <x>/<y>/<z>", CommandStatus::Error };
        }

        glm::vec3 pos = StringUtils::fromString<glm::vec3>(args[0]);
        g_Game->teleport(pos);

        return { "Teleported to: " + StringUtils::toString(pos), CommandStatus::Success };
    });

    g_Console->addAlias("tp", "teleport");

    g_Console->bindProperty("Game.viewDistance", g_Game, &Game::getViewDistance, &Game::setViewDistance);
    g_Console->addAlias("Game.setViewDistance", "Game.viewDistance");
}

void Game::update(float dt)
{
    int width, height;
    g_Window->getSize(&width, &height);

    float aspect_ratio = static_cast<float>(width) /
                         static_cast<float>(height);

    if (_camera.getAspectRatio() != aspect_ratio)
        _camera.setAspectRatio(aspect_ratio);

    if (g_UiMgr->getConsoleUI().isOpen())
        return;

    glm::vec3 move_dir(0.0f);
    glm::vec3 forward = _camera.getForward();
    if (!_cinematic_camera) forward.y = 0.0f;
    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);

    glm::vec3 right = _camera.getRight();
    if (!_cinematic_camera) right.y = 0.0f;
    if (glm::length(right) > 0.0f)
        right = glm::normalize(right);

    if (g_Input->isKeyDown(MOVE_FORWARD_KEY))  move_dir += forward;
    if (g_Input->isKeyDown(MOVE_BACKWARD_KEY)) move_dir -= forward;
    if (g_Input->isKeyDown(MOVE_LEFT_KEY))     move_dir -= right;
    if (g_Input->isKeyDown(MOVE_RIGHT_KEY))    move_dir += right;
    if (g_Input->isKeyDown(MOVE_UP_KEY))       move_dir += glm::vec3(0.0f, 1.0f, 0.0f);
    if (g_Input->isKeyDown(MOVE_DOWN_KEY))     move_dir += glm::vec3(0.0f, -1.0f, 0.0f);

    glm::vec3 target_velocity(0.0f);
    if (glm::length(move_dir) > 0.0f)
    {
        target_velocity = glm::normalize(move_dir) * _camera_speed;
    }

    if (_cinematic_camera)
    {
        float move_smoothness = 5.0f; 
        float move_factor = 1.0f - std::exp(-move_smoothness * dt);
        _camera_velocity = glm::mix(_camera_velocity, target_velocity, move_factor);
    }
    else
    {
        _camera_velocity = target_velocity;
    }

    if (glm::length(_camera_velocity) > 0.001f)
    {
        _camera.move(_camera_velocity * dt);
    }
    else
    {
        _camera_velocity = glm::vec3(0.0f);
    }

    if (g_Input->isMouseCaptured())
    {
        int delta_x, delta_y;
        g_Input->getMouseDelta(&delta_x, &delta_y);

        float sensitivity = 0.15f;
        
        static glm::vec3 target_rot = _camera.getRotation();

        if (!_cinematic_camera)
        {
            target_rot = _camera.getRotation();
        }

        target_rot.y -= (float)delta_x * sensitivity;
        target_rot.x += (float)delta_y * sensitivity;
        target_rot.x = glm::clamp(target_rot.x, -89.0f, 89.0f);

        if (target_rot.y >= 360.0f) target_rot.y -= 360.0f;
        else if (target_rot.y < 0.0f) target_rot.y += 360.0f;

        if (_cinematic_camera)
        {
            glm::vec3 current_rot = _camera.getRotation();

            float yaw_delta = target_rot.y - current_rot.y;
            if (yaw_delta > 180.0f)  yaw_delta -= 360.0f;
            if (yaw_delta < -180.0f) yaw_delta += 360.0f;

            float smoothness = 6.0f; 
            float factor = 1.0f - std::exp(-smoothness * dt);

            glm::vec3 new_rot;
            new_rot.x = glm::mix(current_rot.x, target_rot.x, factor);
            new_rot.y = current_rot.y + yaw_delta * factor;

            if (new_rot.y >= 360.0f) new_rot.y -= 360.0f;
            else if (new_rot.y < 0.0f) new_rot.y += 360.0f;

            float strafe_roll = 0.0f;
            if (g_Input->isKeyDown(MOVE_LEFT_KEY))  strafe_roll += 1.5f;
            if (g_Input->isKeyDown(MOVE_RIGHT_KEY)) strafe_roll -= 1.5f;

            float target_roll = (-(float)delta_x * 0.12f) + strafe_roll; 
            target_roll = glm::clamp(target_roll, -3.5f, 3.5f);
            
            new_rot.z = glm::mix(current_rot.z, target_roll, 4.0f * dt);

            _camera.setRotation(new_rot);
        }
        else
        {
            target_rot.z = 0.0f;
            _camera.setRotation(target_rot);
        }
    }
}
void Game::onEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    {
        if (event.key.repeat) break;

        if (event.key.scancode == SDL_SCANCODE_ESCAPE)
        {
            g_Input->setMouseCaptured(!g_Input->isMouseCaptured());
        }
        else if (event.key.scancode == SDL_SCANCODE_F1)
        {
            g_Renderer->setWireframeEnabled(!g_Renderer->isWireframeEnabled());
        }
        else if (event.key.scancode == SDL_SCANCODE_F3)
        {
            auto& stats_overlay = g_UiMgr->getStatsOverlayUI();
            stats_overlay.toggle();
        }
        else if (event.key.scancode == SDL_SCANCODE_F5)
        {
            g_ShaderMgr->reloadAll();
        }
        else if (event.key.scancode == SDL_SCANCODE_F11)
        {
            _fullscreen = !_fullscreen;
            
            // TODO: Replace with window.setFullscreen(bool fullscreen)
            SDL_SetWindowFullscreen(g_Window->getHandle(), _fullscreen);
        }
        else if (event.key.scancode == SDL_SCANCODE_GRAVE)
        {
            auto& console_ui = g_UiMgr->getConsoleUI();
            console_ui.toggle();
        }
        else if (event.key.scancode == SDL_SCANCODE_C)
        {
            _cinematic_camera = !_cinematic_camera;
            
            LOG_INFO("Game::onEvent: Cinematic camera %s!", _cinematic_camera ? "enabled" : "disabled");
        }
    } break;
    case SDL_EVENT_MOUSE_WHEEL:
    {
        const float speed_change = 2.0f;
        if (event.wheel.y > 0)
        {
            _camera_speed += speed_change;
        }
        else if (event.wheel.y < 0)
        {
            _camera_speed -= speed_change;
            if (_camera_speed < 0.1f)
                _camera_speed = 0.1f;
        }
    } break;
    case SDL_EVENT_WINDOW_RESIZED:
    {
        g_Renderer->setViewport(0, 0, (int)event.window.data1, (int)event.window.data2);
    } break;
    default: break;
    }
}

bool Game::loadLevel(const std::string& name)
{
    LOG_INFO("Game::loadLevel: Loading level '%s'...", name.c_str());

    bool success = g_VFS->mountProvider(std::make_shared<RFAProvider>(
        std::string(GAME_DATA_DIR) + "/bf1942/Archives/bf1942/levels/" + name + ".rfa"
    ));

    if (!success)
    {
        LOG_ERROR("Game::loadLevel: Failed to mount level archive!");
        return false;
    }

    if (!_objs_loaded)
    {
        if (!loadGameObjs())
        {
            LOG_INFO("Game::loadLevel: Failed to load game objects!");
            return false;
        }
    }

    g_ScriptMgr->execCon("bf1942/levels/" + name + "/Init.con");
    g_ScriptMgr->execCon("bf1942/levels/" + name + "/StaticObjects.con");

    // if (!Geometry::uploadAll())
    // {
    //     LOG_ERROR("Game::loadLevel: Failed to upload geometries to GPU!");
    //     return false;
    // }

    g_World->getWater().init();

    LOG_INFO("Game::loadLevel: Level '%s' loaded!", name.c_str());

    return true;
}

void Game::teleport(const glm::vec3& position)
{
    _camera.setPosition(position);
}

float Game::getViewDistance() const
{
    return _camera.getFarPlane();
}

void Game::setViewDistance(float distance)
{
    _camera.setFarPlane(distance);
}

bool Game::loadGameObjs()
{
    LOG_INFO("Game::loadGameObjs: Loading game objects...");

    auto t_start = std::chrono::steady_clock::now();

    bool success = g_VFS->mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/Objects.rfa"));
    if (!success)
    {
        LOG_ERROR("Game::loadGameObjs: Failed to mount 'Objects.rfa' archive!");
        return false;
    }

    std::vector<std::string> object_paths = g_VFS->listFiles("Objects/");

    std::vector<std::string> con_paths;
    for (auto& path : object_paths) {
        if (path.ends_with(".con")) con_paths.push_back(path);
    }

    auto t_mount_done = std::chrono::steady_clock::now();

    std::vector<std::future<bool>> futures;
    futures.reserve(con_paths.size());

    for (const auto& path : con_paths)
        futures.push_back(g_ScriptMgr->execConAsync(path));

    bool all_ok = true;
    for (auto& f : futures)
        if (!f.get()) all_ok = false;

    if (!all_ok)
    {
        LOG_ERROR("Game::loadGameObjs: One or more .con files failed to load!");
        return false;
    }
    
    auto t_end = std::chrono::steady_clock::now();

    auto mount_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_mount_done - t_start).count();
    auto load_ms  = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_mount_done).count();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();

    LOG_INFO("Game::loadGameObjs: Mounted archive + listed %zu .con files in %lld ms",
              con_paths.size(), mount_ms);
    LOG_INFO("Game::loadGameObjs: Loaded %zu objects in %lld ms (async)",
              con_paths.size(), load_ms);
    LOG_INFO("Game::loadGameObjs: Total: %lld ms", total_ms);

    _objs_loaded = true;

    return true;
}
