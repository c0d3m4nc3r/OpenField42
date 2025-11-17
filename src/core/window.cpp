#include "core/window.h"
#include "core/config.h"
#include "core/game.h"
#include "core/input.h"
#include "core/log.h"

#include "glad/glad.h"

#include <SDL3/SDL_events.h>

Window window;

bool Window::init()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    Uint64 window_flags = SDL_WINDOW_OPENGL;
    if (WINDOW_FULLSCREEN)
        window_flags |= SDL_WINDOW_FULLSCREEN;
    if (WINDOW_RESIZABLE)
        window_flags |= SDL_WINDOW_RESIZABLE;

    // fullscreen + borderless = no quit event from i3wm for some reason
    if (WINDOW_BORDERLESS && !WINDOW_FULLSCREEN)
        window_flags |= SDL_WINDOW_BORDERLESS;

    this->window = SDL_CreateWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
    if (!window)
    {
        LOG_ERROR("Window::init: Failed to create window! : %s", SDL_GetError());
        return false;
    }

    gl_context = SDL_GL_CreateContext(this->window);
    if (!gl_context)
    {
        LOG_ERROR("Window::init: Failed to create OpenGL context! : %s", SDL_GetError());
        return false;
    }
    SDL_GL_MakeCurrent(this->window, gl_context);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        LOG_ERROR("Window::init: Failed to initialize GLAD!");
        return false;
    }

    SDL_GL_SetSwapInterval(WINDOW_VSYNC_ON ? 1 : 0);

    return true;
}

void Window::shutdown()
{
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(this->window);
}

void Window::pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        game.onEvent(event);
        input.onEvent(event);
    }
}

void Window::update()
{
    SDL_GL_SwapWindow(this->window);
}

SDL_Window* Window::getWindow() const
{
    return this->window;
}

SDL_GLContext Window::getGLContext() const
{
    return this->gl_context;
}

bool Window::getSize(int* width, int* height) const
{
    return SDL_GetWindowSize(this->window, width, height);
}

float Window::getAspectRatio() const
{
    int width, height;
    if (getSize(&width, &height) && width != 0 && height != 0)
        return static_cast<float>(width) / static_cast<float>(height);
    return 1.0f;
}