#pragma once

struct EngineStats;
class Window;
class Renderer;
class DebugUI
{
public:

    void init();
    void onEvent(const SDL_Event& event);
    void render(const EngineStats& stats);

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool enabled) { _enabled = enabled; }

private:
    bool _enabled = false;
};
