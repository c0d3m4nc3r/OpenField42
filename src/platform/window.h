#pragma once

#include <SDL3/SDL_video.h>

class Window
{
public:

    bool init();
    void shutdown();
    void swapBuffers();

    SDL_Window* getHandle() const { return _handle; }
    SDL_GLContext getGLContext() const { return _gl_context; }

    bool getSize(int* width, int* height) const;
    float getAspectRatio() const;

private:

    SDL_Window* _handle = nullptr;
    SDL_GLContext _gl_context = nullptr;
};
