#pragma once

struct EngineStats
{
    float fps, delta_time;
    float update_time_ms;
    float render_time_ms;
    float total_frame_time_ms;
};

class Game;
class Engine
{
public:

    bool init(int argc, char* argv[]);
    void shutdown();
    void tick(float dt, float fps, uint64_t frequency);

    float getFPS() const { return _stats.fps; }
    float getDeltaTime() const { return _stats.delta_time; }

    bool isRunning() const { return _is_running; }

private:

    EngineStats _stats;

    bool _is_running = false;

    void update(float dt);
    void render();
    
};
