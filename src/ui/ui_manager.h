#pragma once

#include "ui/stats_overlay.h"

class UIManager
{
public:

    void init();
    void onEvent(const SDL_Event& event);
    void render(const EngineStats& stats);

    StatsOverlayUI& getStatsOverlay() { return _stats_overlay; }

private:

    StatsOverlayUI _stats_overlay;
    
};
