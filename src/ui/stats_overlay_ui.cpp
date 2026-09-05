#include "ui/stats_overlay_ui.h"

#include "core/engine.h"
#include "core/globals.h"
#include "render/camera.h"
#include "render/renderer.h"

#include "imgui.h"

void StatsOverlayUI::render(const EngineStats& stats)
{
    if (!_enabled) return;
    
    ImVec2 pos = ImVec2(0, 0);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowBgAlpha(0.05f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Stats:", nullptr, flags);

    ImGui::Text("FPS: %.1f", stats.fps);
    ImGui::Text("Frame Time: %.2f ms", stats.delta_time * 1000.0f);

    ImGui::Separator();
    ImGui::Text("Performance Profiling:");
    ImGui::Text("\tUpdate: %.2f ms", stats.update_time_ms);
    ImGui::Text("\tRender: %.2f ms", stats.render_time_ms);
    ImGui::Text("\tTotal Frame: %.2f ms", stats.total_frame_time_ms);

    ImGui::Separator();
    ImGui::Text("Frame Time Distribution:");
    ImGui::Text("\tUpdate: [%.2f%%]", (stats.update_time_ms / stats.total_frame_time_ms) * 100.0f);
    ImGui::Text("\tRender: [%.2f%%]", (stats.render_time_ms / stats.total_frame_time_ms) * 100.0f);

    const auto& render_stats = g_Renderer->getStats(); 

    ImGui::Separator();
    ImGui::Text("Renderer Statistics:");
    ImGui::Text("\tMeshes Rendered: %zu", render_stats.meshes_rendered);
    ImGui::Text("\tMeshes Culled: %zu", render_stats.meshes_culled);
    ImGui::Text("\tPolygons Rendered: %zu", render_stats.polygons_rendered);
    ImGui::Text("\tPolygons Culled: %zu", render_stats.polygons_culled);

    size_t total_meshes = render_stats.meshes_rendered + render_stats.meshes_culled;
    if (total_meshes > 0) 
    {
        float culling_efficiency = (static_cast<float>(render_stats.meshes_culled) / total_meshes) * 100.0f;
        ImGui::Text("\tCulling Efficiency: %.1f%%", culling_efficiency);
        
        ImGui::Text("\tVisible/Culled Ratio: ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%zu/%zu", 
                        render_stats.meshes_rendered, render_stats.meshes_culled);
    }

    ImGui::Separator();
    ImGui::Text("Per-pass Statistics:");

    for (size_t i = 0; i < static_cast<size_t>(RenderPass::Type::Count); i++)
    {
        auto type = static_cast<RenderPass::Type>(i);
        auto* pass = g_Renderer->getPass(type);
        const auto& pass_stats = pass->getStats();

        ImGui::Text("\t%s: ", passTypeToString(type).c_str());
        ImGui::Text("\t\tMeshes rendered: %zu", pass_stats.meshes_rendered);
        ImGui::Text("\t\tPolygons rendered: %zu", pass_stats.polygons_rendered);

    }

    ImGui::Separator();

    ImGui::Text("Memory Usage:");
    ImGui::Text("\tTextures: %zu / %.2f MB", g_TextureMgr->count(), g_TextureMgr->getMemoryUsage() / (1024.0f * 1024.0f));

    Camera* camera = g_Renderer->getCamera();

    ImGui::Separator();
    const glm::vec3& camera_pos = camera->getPosition();
    const glm::vec3& camera_rot = camera->getRotation();
    ImGui::Text("Camera:");
    ImGui::Text("\tPosition: X:%.2f, Y:%.2f, Z:%.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("\tRotation: Pitch:%.2f, Yaw:%.2f", camera_rot.x, camera_rot.y);
    ImGui::End();
    ImGui::PopStyleVar();
}