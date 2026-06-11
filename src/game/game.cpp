#include "game/game.h"

#include "core/console.h"
#include "utils/log.h"
#include "platform/input.h"
#include "platform/window.h"
#include "render/renderer.h"
#include "vfs/vfs.h"
#include "world/water.h"
#include "world/world.h"


bool Game::init()
{
    LOG_INFO("Game::init: Initializing game...");

    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/standardMesh.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/StandardMesh_001.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture_001.rfa"));

    auto& input = _engine.getInput();

    input.setMouseCaptured(true);

    const float WORLD_SIZE = 2048.0f;
    glm::vec3 world_center(WORLD_SIZE/2.0f);
    world_center.y = 75.0f;

    _camera.setPosition(world_center);

    auto& renderer = _engine.getRenderer();
    renderer.setCamera(&_camera);

    LOG_INFO("Game::init: Game initialized!");

    return true;
}

void Game::update(float dt)
{
    float move_speed = _camera_speed * dt;

    glm::vec3 move_dir(0.0f);
    glm::vec3 forward = _camera.getForward();
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);

    glm::vec3 right = _camera.getRight();
    right.y = 0.0f;
    if (glm::length(right) > 0.0f)
        right = glm::normalize(right);

    auto& input = _engine.getInput();

    if (input.isKeyDown(MOVE_FORWARD_KEY))  move_dir += forward;
    if (input.isKeyDown(MOVE_BACKWARD_KEY)) move_dir -= forward;
    if (input.isKeyDown(MOVE_LEFT_KEY))     move_dir -= right;
    if (input.isKeyDown(MOVE_RIGHT_KEY))    move_dir += right;
    if (input.isKeyDown(MOVE_UP_KEY))       move_dir += glm::vec3(0.0f, 1.0f, 0.0f);
    if (input.isKeyDown(MOVE_DOWN_KEY))     move_dir += glm::vec3(0.0f, -1.0f, 0.0f);

    if (glm::length(move_dir) > 0.0f)
    {
        move_dir = glm::normalize(move_dir);
        _camera.move(move_dir * _camera_speed * dt);
    }
    
    if (input.isMouseCaptured())
    {
        int delta_x, delta_y;
        input.getMouseDelta(&delta_x, &delta_y);

        float sensitivity = 0.15f;
        glm::vec3 rot = _camera.getRotation();

        rot.y -= (float)delta_x * sensitivity;
        rot.x += (float)delta_y * sensitivity;
        rot.x = glm::clamp(rot.x, -89.0f, 89.0f);

        if (rot.y > 360.0f) rot.y -= 360.0f;
        else if (rot.y < 0.0f) rot.y += 360.0f;

        _camera.setRotation(rot);
    }

    auto& window = _engine.getWindow();

    int width, height;
    window.getSize(&width, &height);

    float aspect_ratio = static_cast<float>(width) /
                         static_cast<float>(height);

    if (_camera.getAspectRatio() != aspect_ratio)
        _camera.setAspectRatio(aspect_ratio);
}

void Game::onEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
    {
        if (event.key.scancode == SDL_SCANCODE_ESCAPE)
        {
            auto& input = _engine.getInput();
            input.setMouseCaptured(!input.isMouseCaptured());
        }
        else if (event.key.scancode == SDL_SCANCODE_F1)
        {
            auto& renderer = _engine.getRenderer();
            renderer.setWireframeEnabled(!renderer.isWireframeEnabled());
        }
        else if (event.key.scancode == SDL_SCANCODE_F3)
        {
            _engine.toggleDebugUI();
        }
        else if (event.key.scancode == SDL_SCANCODE_F11)
        {
            _fullscreen = !_fullscreen;
            
            auto& window = _engine.getWindow();

            // TODO: Replace with window.setFullscreen(bool fullscreen)
            SDL_SetWindowFullscreen(window.getHandle(), _fullscreen);
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
    default: break;
    }
}

bool Game::loadLevel(const std::string& name)
{
    LOG_INFO("Game::loadLevel: Loading level '%s'...", name.c_str());

    bool success = VFS::mountProvider(std::make_shared<RFAProvider>(
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

    auto& console = _engine.getConsole();
    console.execFile("bf1942/levels/" + name + "/Init.con");
    console.execFile("bf1942/levels/" + name + "/StaticObjects.con");

    if (!Geometry::uploadAll())
    {
        LOG_ERROR("Game::loadLevel: Failed to upload geometries to GPU!");
        return false;
    }

    auto& world = _engine.getWorld();
    world.getWater().init(world.getTerrain());

    LOG_INFO("Game::loadLevel: Level '%s' loaded!", name.c_str());

    return true;
}

void Game::setViewDistance(float distance)
{
    _camera.setFarPlane(distance);
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

    auto& console = _engine.getConsole();

    for (const auto& path : object_paths)
    {
        if (!path.ends_with(".con")) continue;
        
        if (!console.execFile(path))
        {
            LOG_ERROR("Game::loadGameObjs: Failed to load game objects: Error in '%s'!", path.c_str());
            return false;
        }
    }

    LOG_INFO("Game::loadGameObjs: Game objects loaded successfully!");

    _objs_loaded = true;

    return true;
}
