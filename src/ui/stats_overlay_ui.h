#pragma once

struct EngineStats;
class StatsOverlayUI
{
public:

    void render(const EngineStats& stats);

    void toggle() { _enabled = !_enabled; }

private:

    bool _enabled = false;
};
