#include "core/debugui.h"
#include "core/engine.h"
#include "platform/window.h"
#include "render/camera.h"
#include "render/renderer.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

void DebugUI::init(const Window& window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForOpenGL(window.getHandle(), window.getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330");

    LOG_INFO("DebugUI::init: Debug UI initialized!");
}

void DebugUI::onEvent(const SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
}

void DebugUI::render(const EngineStats& stats, const Renderer& renderer)
{
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

    ImGui::Text("FPS: %.1f", stats.fps);
    ImGui::Text("Frame Time: %.2f ms", stats.delta_time * 1000.0f);

    ImGui::Separator();
    ImGui::Text("Performance Profiling:");
    ImGui::Text("Update: %.2f ms", stats.update_time_ms);
    ImGui::Text("Render: %.2f ms", stats.render_time_ms);
    ImGui::Text("Total Frame: %.2f ms", stats.total_frame_time_ms);

    ImGui::Separator();
    ImGui::Text("Frame Time Distribution:");
    ImGui::Text("Update: [%.2f%%]", (stats.update_time_ms / stats.total_frame_time_ms) * 100.0f);
    ImGui::Text("Render: [%.2f%%]", (stats.render_time_ms / stats.total_frame_time_ms) * 100.0f);

    const auto& render_stats = renderer.getStats(); 

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

    Camera* camera = renderer.getCamera();

    ImGui::Separator();
    const glm::vec3& camera_pos = camera->getPosition();
    const glm::vec3& camera_rot = camera->getRotation();
    ImGui::Text("Camera:");
    ImGui::Text("\tPosition: X:%.2f, Y:%.2f, Z:%.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("\tRotation: Pitch:%.2f, Yaw:%.2f", camera_rot.x, camera_rot.y);
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}