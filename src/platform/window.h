#pragma once

#include <SDL3/SDL_video.h>

class Window
{
public:

    bool init();
    void shutdown();
    void pollEvents();
    void update();

    SDL_Window* getHandle() const;
    SDL_GLContext getGLContext() const;

    bool getSize(int* width, int* height) const;
    float getAspectRatio() const;

private:

    SDL_Window* handle = nullptr;
    SDL_GLContext gl_context = nullptr;
};

extern Window g_Window;