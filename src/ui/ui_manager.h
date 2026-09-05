#pragma once

#include "ui/console_ui.h"
#include "ui/stats_overlay_ui.h"

class UIManager
{
public:

    void init();
    void onEvent(const SDL_Event& event);
    void render(const EngineStats& stats);

    ConsoleUI& getConsoleUI() { return _console_ui; }
    StatsOverlayUI& getStatsOverlayUI() { return _stats_overlay_ui; }

private:

    ConsoleUI _console_ui;
    StatsOverlayUI _stats_overlay_ui;
    
};
