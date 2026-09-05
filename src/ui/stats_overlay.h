#pragma once

struct EngineStats;
class StatsOverlayUI
{
public:

    void render(const EngineStats& stats);

    bool isEnabled() const { return _enabled; }
    void setEnabled(bool enabled) { _enabled = enabled; }

private:

    bool _enabled = false;
};
