#include "core/game.h"

#include "core/console.h"
#include "world/sky.h"
#include "utils/log.h"
#include "platform/input.h"
#include "platform/window.h"
#include "render/renderer.h"
#include "object/object.h"
#include "vfs/vfs.h"
#include "world/water.h"

#include "glad/glad.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>

Game g_Game;

bool Game::init()
{
    LOG_INFO("Game::init: Initializing game...");
    
    // Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LOG_ERROR("Game::init: Failed to initialize SDL! : %s", SDL_GetError());
        return false;
    }

    // Initialize window
    if (!g_Window.init())
    {
        LOG_ERROR("Game::init: Failed to initialize window!");
        return false;
    }

    // Mount assets folder
    VFS::mountProvider(std::make_shared<FolderProvider>("assets"));

    // Initialize renderer
    if (!g_Renderer.init())
    {
        LOG_ERROR("Game::init: Failed to initialize renderer!");
        return false;
    }

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForOpenGL(g_Window.getHandle(), g_Window.getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330");

    // Mount game archives
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/standardMesh.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/StandardMesh_001.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture_001.rfa"));

    // Initialize console
    g_Console.init();

    // Initialize renderer
    g_Renderer.setCamera(&_camera);

    // Load all game objects from "Objects.rfa"
    if (!loadGameObjs())
    {
        LOG_ERROR("Game::init: Failed to load game objects!");
        return false;
    }

    // Capture mouse
    g_Input.setMouseCaptured(true);

    // Move camera to center and setup far plane

    // TODO: Get world size from Terrain.con
    const float WORLD_SIZE = 2048.0f;
    glm::vec3 world_center(WORLD_SIZE/2.0f);
    // glm::vec3 world_center(0.0f);
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

    SDL_Quit();

    LOG_INFO("Game::shutdown: Game shutdown!");
}

void Game::tick(float delta_time, float fps, Uint64 frequency)
{
    _delta_time = delta_time;
    _fps = fps;

    Uint64 frame_start = SDL_GetPerformanceCounter();
    
    Uint64 update_start = SDL_GetPerformanceCounter();
    update();
    Uint64 update_end = SDL_GetPerformanceCounter();
    _update_time_ms = (float)(update_end - update_start) / frequency * 1000.0f;
    
    Uint64 render_start = SDL_GetPerformanceCounter();
    render();
    Uint64 render_end = SDL_GetPerformanceCounter();
    _render_time_ms = (float)(render_end - render_start) / frequency * 1000.0f;
    
    Uint64 frame_end = SDL_GetPerformanceCounter();
    _total_frame_time_ms = (float)(frame_end - frame_start) / frequency * 1000.0f;
}

void Game::onEvent(const SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
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

    float move_speed = _camera_speed * _delta_time;

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
        _camera.move(move_dir * _camera_speed * _delta_time);
    }
    
    if (g_Input.isMouseCaptured())
    {
        int delta_x, delta_y;
        g_Input.getMouseDelta(&delta_x, &delta_y);

        float sensitivity = 0.15f;
        glm::vec3 rot = _camera.getRotation();

        rot.y += (float)delta_x * sensitivity;
        rot.x -= (float)delta_y * sensitivity;
        rot.x = glm::clamp(rot.x, -89.0f, 89.0f);

        if (rot.y > 360.0f) rot.y -= 360.0f;
        else if (rot.y < 0.0f) rot.y += 360.0f;

        _camera.setRotation(rot);
    }

    if (_camera.getFarPlane() != view_distance)
        _camera.setFarPlane(view_distance);

    for (auto& obj : Object::registry)
        obj->update(_delta_time);
}

void Game::render()
{
    int width, height;
    g_Window.getSize(&width, &height);

    float aspect_ratio = static_cast<float>(width) /
                         static_cast<float>(height);

    if (_camera.getAspectRatio() != aspect_ratio)
        _camera.setAspectRatio(aspect_ratio);
    
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImVec2 pos = ImVec2(0, 0);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowBgAlpha(0.05f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Debug Info", nullptr, flags);

    ImGui::Text("FPS: %.1f", _fps);
    ImGui::Text("Frame Time: %.2f ms", _delta_time * 1000.0f);

    ImGui::Separator();
    ImGui::Text("Performance Profiling:");
    ImGui::Text("Update: %.2f ms", _update_time_ms);
    ImGui::Text("Render: %.2f ms", _render_time_ms);
    ImGui::Text("Total Frame: %.2f ms", _total_frame_time_ms);

    ImGui::Separator();
    ImGui::Text("Frame Time Distribution:");
    ImGui::Text("Update: [%.2f%%]", (_update_time_ms / _total_frame_time_ms) * 100.0f);
    ImGui::Text("Render: [%.2f%%]", (_render_time_ms / _total_frame_time_ms) * 100.0f);

    const auto& render_stats = g_Renderer.getStats(); 

    ImGui::Separator();
    ImGui::Text("Renderer Statistics:");
    ImGui::Text("Meshes Rendered: %zu", render_stats.meshes_rendered);
    ImGui::Text("Meshes Culled: %zu", render_stats.meshes_culled);
    ImGui::Text("Polygons Rendered: %zu", render_stats.polygons_rendered);
    ImGui::Text("Polygons Culled: %zu", render_stats.polygons_culled);

    size_t total_meshes = render_stats.meshes_rendered + render_stats.meshes_culled;
    if (total_meshes > 0) 
    {
        float culling_efficiency = (static_cast<float>(render_stats.meshes_culled) / total_meshes) * 100.0f;
        ImGui::Text("Culling Efficiency: %.1f%%", culling_efficiency);
        
        ImGui::Text("Visible/Culled Ratio: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%zu/%zu", 
                        render_stats.meshes_rendered, render_stats.meshes_culled);
    }

    ImGui::Separator();
    const glm::vec3& camera_pos = _camera.getPosition();
    const glm::vec3& camera_rot = _camera.getRotation();
    ImGui::Text("Camera:");
    ImGui::Text("\tPosition: X:%.2f, Y:%.2f, Z:%.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("\tRotation: Pitch:%.2f, Yaw:%.2f", camera_rot.x, camera_rot.y);
    ImGui::Text("\tSpeed: %.2f", _camera_speed);

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

    LOG_INFO("Game::loadLevel: Level '%s' loaded!", name.c_str());

    return true;
}
