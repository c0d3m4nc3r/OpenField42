#pragma once

#include <SDL3/SDL_video.h>

class Window
{
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;

public:

    bool init();
    void shutdown();
    void pollEvents();
    void update();

    SDL_Window* getWindow() const;
    SDL_GLContext getGLContext() const;

    bool getSize(int* width, int* height) const;
    float getAspectRatio() const;
};

extern Window window;