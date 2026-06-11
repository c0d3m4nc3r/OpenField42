#include "core/game.h"

#include "core/console.h"
#include "core/debugui.h"
#include "world/sky.h"
#include "utils/log.h"
#include "platform/input.h"
#include "platform/window.h"
#include "render/renderer.h"
#include "object/object.h"
#include "vfs/vfs.h"
#include "world/water.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>

Game g_Game;

bool Game::init()
{
    LOG_INFO("Game::init: Initializing game...");

    if (!g_Window.init())
    {
        LOG_ERROR("Game::init: Failed to initialize window!");
        return false;
    }

    VFS::mountProvider(std::make_shared<FolderProvider>("assets"));

    if (!g_Renderer.init())
    {
        LOG_ERROR("Game::init: Failed to initialize renderer!");
        return false;
    }

    g_Renderer.setCamera(&_camera);

    g_DebugUI.init();

    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/standardMesh.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/StandardMesh_001.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture_001.rfa"));

    g_Console.init();

    if (!loadGameObjs())
    {
        LOG_ERROR("Game::init: Failed to load game objects!");
        return false;
    }

    g_Input.setMouseCaptured(true);

    const float WORLD_SIZE = 2048.0f;
    glm::vec3 world_center(WORLD_SIZE/2.0f);
    world_center.y = 75.0f;

    _camera.setPosition(world_center);

    if (VIEW_DISTANCE != 0)
        view_distance = VIEW_DISTANCE;

    _running = true;

    LOG_INFO("Game::init: Game initialized!");

    return true;
}

void Game::shutdown()
{
    LOG_INFO("Game::shutdown: Shutting down game...");

    g_Water.shutdown();
    g_Renderer.shutdown();
    g_Window.shutdown();

    LOG_INFO("Game::shutdown: Game shutdown!");
}

void Game::tick(float delta_time, float fps, Uint64 frequency)
{
    _stats.delta_time = delta_time;
    _stats.fps = fps;

    Uint64 frame_start = SDL_GetPerformanceCounter();
    
    Uint64 update_start = SDL_GetPerformanceCounter();
    update();
    Uint64 update_end = SDL_GetPerformanceCounter();
    _stats.update_time_ms = (float)(update_end - update_start) / frequency * 1000.0f;
    
    Uint64 render_start = SDL_GetPerformanceCounter();
    render();
    Uint64 render_end = SDL_GetPerformanceCounter();
    _stats.render_time_ms = (float)(render_end - render_start) / frequency * 1000.0f;
    
    Uint64 frame_end = SDL_GetPerformanceCounter();
    _stats.total_frame_time_ms = (float)(frame_end - frame_start) / frequency * 1000.0f;
}

void Game::onEvent(const SDL_Event& event)
{    
    switch (event.type)
    {
    case SDL_EVENT_QUIT:
        {
            _running = false;
        }
        break;
    case SDL_EVENT_KEY_DOWN:
        {
            if (event.key.scancode == SDL_SCANCODE_ESCAPE)
            {
                g_Input.setMouseCaptured(!g_Input.isMouseCaptured());
            }
            else if (event.key.scancode == SDL_SCANCODE_F1)
            {
                g_Renderer.wireframe_mode = !g_Renderer.wireframe_mode;
            }
            else if (event.key.scancode == SDL_SCANCODE_F3)
            {
                _draw_debug_info = !_draw_debug_info;
            }
            else if (event.key.scancode == SDL_SCANCODE_F11)
            {
                _fullscreen = !_fullscreen;

                // TODO: Replace with window.setFullscreen(bool fullscreen)
                SDL_SetWindowFullscreen(g_Window.getHandle(), _fullscreen);
            }
        }
        break;
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
        }
        break;
    default:
        break;
    }
}

void Game::update()
{
    g_Input.update();
    g_Window.pollEvents();

    float move_speed = _camera_speed * _stats.delta_time;

    glm::vec3 move_dir(0.0f);
    glm::vec3 forward = _camera.getForward();
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);

    glm::vec3 right = _camera.getRight();
    right.y = 0.0f;
    if (glm::length(right) > 0.0f)
        right = glm::normalize(right);

    if (g_Input.isKeyDown(MOVE_FORWARD_KEY))  move_dir += forward;
    if (g_Input.isKeyDown(MOVE_BACKWARD_KEY)) move_dir -= forward;
    if (g_Input.isKeyDown(MOVE_LEFT_KEY))     move_dir -= right;
    if (g_Input.isKeyDown(MOVE_RIGHT_KEY))    move_dir += right;
    if (g_Input.isKeyDown(MOVE_UP_KEY))       move_dir += glm::vec3(0.0f, 1.0f, 0.0f);
    if (g_Input.isKeyDown(MOVE_DOWN_KEY))     move_dir += glm::vec3(0.0f, -1.0f, 0.0f);

    if (glm::length(move_dir) > 0.0f)
    {
        move_dir = glm::normalize(move_dir);
        _camera.move(move_dir * _camera_speed * _stats.delta_time);
    }
    
    if (g_Input.isMouseCaptured())
    {
        int delta_x, delta_y;
        g_Input.getMouseDelta(&delta_x, &delta_y);

        float sensitivity = 0.15f;
        glm::vec3 rot = _camera.getRotation();

        rot.y -= (float)delta_x * sensitivity;
        rot.x += (float)delta_y * sensitivity;
        rot.x = glm::clamp(rot.x, -89.0f, 89.0f);

        if (rot.y > 360.0f) rot.y -= 360.0f;
        else if (rot.y < 0.0f) rot.y += 360.0f;

        _camera.setRotation(rot);
    }

    if (_camera.getFarPlane() != view_distance)
        _camera.setFarPlane(view_distance);

    for (auto& obj : Object::registry)
        obj->update(_stats.delta_time);
}

void Game::render()
{
    int width, height;
    g_Window.getSize(&width, &height);

    float aspect_ratio = static_cast<float>(width) /
                         static_cast<float>(height);

    if (_camera.getAspectRatio() != aspect_ratio)
        _camera.setAspectRatio(aspect_ratio);

    g_Renderer.resetStats();

    // Draw all objects
    for (auto& obj : Object::registry)
        obj->draw();

    g_Renderer.flush();
    
    // Draw debug information
    if (!_draw_debug_info)
    {
        g_Window.update();
        return;
    }
    
    g_DebugUI.render(_stats, _camera);

    g_Window.update();
}

bool Game::loadGameObjs()
{
    LOG_INFO("Game::loadGameObjs: Loading game objects...");

    bool success = VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/Objects.rfa"));
    if (!success)
    {
        LOG_ERROR("Game::loadGameObjs: Failed to mount 'Objects.rfa' archive!");
        return false;
    }

    std::vector<std::string> object_paths = VFS::listFiles("Objects/");

    for (const auto& path : object_paths)
    {
        if (!path.ends_with(".con")) continue;
        
        if (!g_Console.execFile(path))
        {
            LOG_ERROR("Game::loadGameObjs: Failed to load game objects: Error in '%s'!", path.c_str());
            return false;
        }
    }

    LOG_INFO("Game::loadGameObjs: Loaded successfully!");

    return true;
}

bool Game::loadLevel(const std::string& name)
{
    LOG_INFO("Game::loadLevel: Loading level '%s'...", name.c_str());

    // 1. Mount level archive
    bool success = VFS::mountProvider(std::make_shared<RFAProvider>(
        std::string(GAME_DATA_DIR) + "/bf1942/Archives/bf1942/levels/" + name + ".rfa"
    ));

    if (!success)
    {
        LOG_ERROR("Game::loadLevel: Failed to mount level archive!");
        return false;
    }

    // 2. Run level init scripts
    g_Console.execFile("bf1942/levels/" + name + "/Init.con");
    g_Console.execFile("bf1942/levels/" + name + "/StaticObjects.con");

    // 3. Upload all geometries
    if (!Geometry::uploadAll())
    {
        LOG_ERROR("Game::loadLevel: Failed to upload geometries to GPU!");
        return false;
    }

    g_Water.init();

    LOG_INFO("Game::loadLevel: Level '%s' loaded!", name.c_str());

    return true;
}
