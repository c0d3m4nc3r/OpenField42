#include "core/game.h"

#include "core/console.h"
#include "world/sky.h"
#include "utils/log.h"
#include "platform/input.h"
#include "platform/window.h"
#include "render/renderer.h"
#include "render/shader.h"
#include "object/object.h"
#include "vfs/vfs.h"

#include "glad/glad.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_timer.h>

Game game;

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
    if (!window.init())
    {
        LOG_ERROR("Game::init: Failed to initialize window!");
        return false;
    }

    // Setup OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForOpenGL(window.getWindow(), window.getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330");

    // Mount folders and game archives
    VFS::mountProvider(std::make_shared<FolderProvider>("assets"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/standardMesh.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/StandardMesh_001.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture.rfa"));
    VFS::mountProvider(std::make_shared<RFAProvider>(std::string(GAME_DATA_DIR) + "/bf1942/Archives/texture_001.rfa"));

    // Initialize console
    console.init();

    // Load internal resources
    if (!loadResources())
    {
        LOG_ERROR("Game::init: Failed to load resources!");
        return false;
    }

    // Initialize renderer
    renderer.setCamera(&camera);

    // Load all game objects from "Objects.rfa"
    if (!loadGameObjs())
    {
        LOG_ERROR("Game::init: Failed to load game objects!");
        return false;
    }

    // Capture mouse
    input.setMouseCaptured(true);

    // Move camera to center and setup far plane

    // TODO: Get world size from Terrain.con
    const float WORLD_SIZE = 2048.0f;
    glm::vec3 world_center(WORLD_SIZE/2.0f);
    // glm::vec3 world_center(0.0f);
    world_center.y = 75.0f;

    camera.setPosition(world_center);

    if (VIEW_DISTANCE != 0)
        view_distance = VIEW_DISTANCE;

    is_running = true;

    LOG_INFO("Game::init: Game initialized!");

    return true;
}

void Game::shutdown()
{
    LOG_INFO("Game::shutdown: Shutting down game...");
    
    main_shader.reset();

    window.shutdown();

    SDL_Quit();

    LOG_INFO("Game::shutdown: Game shutdown!");
}

void Game::tick(float delta_time, float fps, Uint64 frequency)
{
    this->delta_time = delta_time;
    this->fps = fps;

    Uint64 frame_start = SDL_GetPerformanceCounter();
    
    Uint64 update_start = SDL_GetPerformanceCounter();
    update();
    Uint64 update_end = SDL_GetPerformanceCounter();
    update_time_ms = (float)(update_end - update_start) / frequency * 1000.0f;
    
    Uint64 render_start = SDL_GetPerformanceCounter();
    render();
    Uint64 render_end = SDL_GetPerformanceCounter();
    render_time_ms = (float)(render_end - render_start) / frequency * 1000.0f;
    
    Uint64 frame_end = SDL_GetPerformanceCounter();
    total_frame_time_ms = (float)(frame_end - frame_start) / frequency * 1000.0f;
}

float Game::getFPS() const { return fps; }
float Game::getDeltaTime() const { return delta_time; }

bool Game::isRunning() const { return is_running; }

void Game::onEvent(const SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
    switch (event.type)
    {
    case SDL_EVENT_QUIT:
        {
            is_running = false;
        }
        break;
    case SDL_EVENT_KEY_DOWN:
        {
            if (event.key.scancode == SDL_SCANCODE_ESCAPE)
            {
                input.setMouseCaptured(!input.isMouseCaptured());
            }
            else if (event.key.scancode == SDL_SCANCODE_F1)
            {
                renderer.wireframe_mode = !renderer.wireframe_mode;
            }
            else if (event.key.scancode == SDL_SCANCODE_F3)
            {
                draw_debug_info = !draw_debug_info;
            }
            else if (event.key.scancode == SDL_SCANCODE_F5)
            {
                loadResources();
            }
            else if (event.key.scancode == SDL_SCANCODE_F11)
            {
                fullscreen = !fullscreen;

                // TODO: Replace with window.setFullscreen(bool fullscreen)
                SDL_SetWindowFullscreen(window.getWindow(), fullscreen);
            }
        }
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        {
            const float speed_change = 2.0f;
            if (event.wheel.y > 0)
            {
                camera_speed += speed_change;
            }
            else if (event.wheel.y < 0)
            {
                camera_speed -= speed_change;
                if (camera_speed < 0.1f)
                    camera_speed = 0.1f;
            }
        }
        break;
    default:
        break;
    }
}

void Game::update()
{
    input.update();
    window.pollEvents();

    float move_speed = camera_speed * delta_time;

    glm::vec3 move_dir(0.0f);
    glm::vec3 forward = camera.getForward();
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);

    glm::vec3 right = camera.getRight();
    right.y = 0.0f;
    if (glm::length(right) > 0.0f)
        right = glm::normalize(right);

    if (input.isKeyDown(MOVE_FORWARD_KEY))  move_dir += forward;
    if (input.isKeyDown(MOVE_BACKWARD_KEY)) move_dir -= forward;
    if (input.isKeyDown(MOVE_LEFT_KEY))     move_dir -= right;
    if (input.isKeyDown(MOVE_RIGHT_KEY))    move_dir += right;
    if (input.isKeyDown(MOVE_UP_KEY))       move_dir += glm::vec3(0.0f, 1.0f, 0.0f);
    if (input.isKeyDown(MOVE_DOWN_KEY))     move_dir += glm::vec3(0.0f, -1.0f, 0.0f);

    if (glm::length(move_dir) > 0.0f)
    {
        move_dir = glm::normalize(move_dir);
        camera.move(move_dir * camera_speed * delta_time);
    }
    
    if (input.isMouseCaptured())
    {
        int delta_x, delta_y;
        input.getMouseDelta(&delta_x, &delta_y);

        float sensitivity = 0.15f;
        glm::vec3 rot = camera.getRotation();

        rot.y += (float)delta_x * sensitivity;
        rot.x -= (float)delta_y * sensitivity;
        rot.x = glm::clamp(rot.x, -89.0f, 89.0f);

        if (rot.y > 360.0f) rot.y -= 360.0f;
        else if (rot.y < 0.0f) rot.y += 360.0f;

        camera.setRotation(rot);
    }

    if (camera.getFarPlane() != view_distance)
        camera.setFarPlane(view_distance);

    for (auto& obj : Object::registry)
        obj->update(delta_time);
}

void Game::render()
{
    int width, height;
    window.getSize(&width, &height);

    float aspect_ratio = static_cast<float>(width) /
                         static_cast<float>(height);

    if (camera.getAspectRatio() != aspect_ratio)
        camera.setAspectRatio(aspect_ratio);
    
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset renderer stats
    renderer.meshes_culled = 0;
    renderer.meshes_rendered = 0;
    renderer.polygons_culled = 0;
    renderer.polygons_rendered = 0;

    // Draw all objects
    for (auto& obj : Object::registry)
        obj->draw();

    renderer.flush();
    
    // Draw debug information
    if (!draw_debug_info)
    {
        window.update();
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

    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.2f ms", delta_time * 1000.0f);

    ImGui::Separator();
    ImGui::Text("Performance Profiling:");
    ImGui::Text("Update: %.2f ms", update_time_ms);
    ImGui::Text("Render: %.2f ms", render_time_ms);
    ImGui::Text("Total Frame: %.2f ms", total_frame_time_ms);

    ImGui::Separator();
    ImGui::Text("Frame Time Distribution:");
    ImGui::Text("Update: [%.2f%%]", (update_time_ms / total_frame_time_ms) * 100.0f);
    ImGui::Text("Render: [%.2f%%]", (render_time_ms / total_frame_time_ms) * 100.0f);

    ImGui::Separator();
    ImGui::Text("Renderer Statistics:");
    ImGui::Text("Meshes Rendered: %zu", renderer.meshes_rendered);
    ImGui::Text("Meshes Culled: %zu", renderer.meshes_culled);
    ImGui::Text("Polygons Rendered: %zu", renderer.polygons_rendered);
    ImGui::Text("Polygons Culled: %zu", renderer.polygons_culled);

    size_t total_meshes = renderer.meshes_rendered + renderer.meshes_culled;
    if (total_meshes > 0) 
    {
        float culling_efficiency = (static_cast<float>(renderer.meshes_culled) / total_meshes) * 100.0f;
        ImGui::Text("Culling Efficiency: %.1f%%", culling_efficiency);
        
        ImGui::Text("Visible/Culled Ratio: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%zu/%zu", 
                        renderer.meshes_rendered, renderer.meshes_culled);
    }

    ImGui::Separator();
    const glm::vec3& camera_pos = camera.getPosition();
    const glm::vec3& camera_rot = camera.getRotation();
    ImGui::Text("Camera:");
    ImGui::Text("\tPosition: X:%.2f, Y:%.2f, Z:%.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("\tRotation: Pitch:%.2f, Yaw:%.2f", camera_rot.x, camera_rot.y);
    ImGui::Text("\tSpeed: %.2f", camera_speed);

    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    window.update();
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
        
        if (!console.execFile(path))
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
    console.execFile("bf1942/levels/" + name + "/Init.con");
    console.execFile("bf1942/levels/" + name + "/StaticObjects.con");

    // 3. Upload all geometries
    if (!Geometry::uploadAll())
    {
        LOG_ERROR("Game::loadLevel: Failed to upload geometries to GPU!");
        return false;
    }

    LOG_INFO("Game::loadLevel: Level '%s' loaded!", name.c_str());

    return true;
}

bool Game::loadResources()
{
    LOG_INFO("Game::loadResources: Loading internal resources...");

    main_shader = Shader::load(
        "shaders/main.vs",
        "shaders/main.fs"
    );

    if (!main_shader)
    {
        LOG_ERROR("Game::loadResources: Failed to load main shader!");
        return false;
    }

    renderer.setShader(main_shader);

    LOG_INFO("Game::loadResources: Loaded successfully!");

    return true;
}
