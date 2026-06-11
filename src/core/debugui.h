#pragma once

struct EngineStats;
class Window;
class Renderer;
class DebugUI
{
public:

    void init(const Window& window);
    void onEvent(const SDL_Event& event);
    void render(const EngineStats& stats, const Renderer& renderer);
};
