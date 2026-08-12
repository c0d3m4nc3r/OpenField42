#pragma once

struct EngineStats
{
    float fps, delta_time;
    float update_time_ms;
    float render_time_ms;
    float total_frame_time_ms;
};

class Window;
class Renderer;
class Input;
class Console;
class DebugUI;
class GeometryManager;
class ShaderManager;
struct World;

class Game;
class Engine
{
public:

    Engine();
    ~Engine();

    bool init(int argc, char* argv[]);
    void shutdown();
    void tick(float dt, float fps, uint64_t frequency);

    void toggleDebugUI() { _debug_ui_enabled = !_debug_ui_enabled; }

    Window& getWindow() const { return *_window; }
    Renderer& getRenderer() const { return *_renderer; }
    Input& getInput() const { return *_input; }
    Console& getConsole() const { return *_console; }
    World& getWorld() const { return *_world; }
    GeometryManager& getGeometryMgr() const { return *_geometry_mgr; }
    ShaderManager& getShaderMgr() const { return *_shader_mgr; }

    Game& getGame() const { return *_game; }

    float getFPS() const { return _stats.fps; }
    float getDeltaTime() const { return _stats.delta_time; }

    bool isRunning() const { return _is_running; }

private:

    std::unique_ptr<Window> _window;
    std::unique_ptr<Renderer> _renderer;
    std::unique_ptr<Input> _input;
    std::unique_ptr<Console> _console;
    std::unique_ptr<World> _world;
    std::unique_ptr<DebugUI> _debug_ui;

    std::unique_ptr<GeometryManager> _geometry_mgr;
    std::unique_ptr<ShaderManager> _shader_mgr;

    std::unique_ptr<Game> _game;

    EngineStats _stats;

    bool _debug_ui_enabled = false;
    bool _is_running = false;

    void update(float dt);
    void render();
    
};
