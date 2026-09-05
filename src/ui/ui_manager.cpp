#include "ui/ui_manager.h"

#include "core/globals.h"
#include "platform/window.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

void UIManager::init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForOpenGL(g_Window->getHandle(), g_Window->getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330");

    LOG_INFO("UIManager::init: UI manager initialized!");
}

void UIManager::onEvent(const SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
}

void UIManager::render(const EngineStats& stats)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    _stats_overlay_ui.render(stats);
    _console_ui.render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}