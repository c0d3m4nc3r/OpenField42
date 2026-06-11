#pragma once

struct EngineStats;
class Camera;
class DebugUI
{
public:

    void init();
    void onEvent(const SDL_Event& event);
    void render(const EngineStats& stats, const Camera& camera);
};

extern DebugUI g_DebugUI;
