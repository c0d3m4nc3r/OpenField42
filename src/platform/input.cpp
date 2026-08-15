#include "platform/input.h"

#include "core/globals.h"
#include "platform/window.h"

void Input::onEvent(const SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_EVENT_KEY_DOWN:
            _keys[static_cast<Key>(event.key.scancode)] = true;
            break;
        case SDL_EVENT_KEY_UP:
            _keys[static_cast<Key>(event.key.scancode)] = false;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            _mouse_buttons.set(static_cast<size_t>(event.button.button), true);
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            _mouse_buttons.set(static_cast<size_t>(event.button.button), false);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            _mouse_pos = glm::vec2(event.motion.x, event.motion.y);
            _mouse_delta += glm::vec2(event.motion.xrel, event.motion.yrel);
            break;
        default:
            break;
    }
}

void Input::update()
{
    _prev_keys = _keys;
    _prev_mouse_buttons = _mouse_buttons;
    _mouse_delta = {};
}

bool Input::isKeyUp(Key key) const
{
    return !getKeyState(key);
}

bool Input::isKeyDown(Key key) const
{
    return getKeyState(key);
}

bool Input::isKeyPressed(Key key) const
{
    return getKeyState(key) && !getPrevKeyState(key);
}

bool Input::isKeyReleased(Key key) const
{
    return !getKeyState(key) && getPrevKeyState(key);
}

bool Input::isMouseButtonUp(MouseButton button) const
{
    return !getMouseButtonState(button);
}

bool Input::isMouseButtonDown(MouseButton button) const
{
    return getMouseButtonState(button);
}

bool Input::isMouseButtonPressed(MouseButton button) const
{
    return getMouseButtonState(button) && !getPrevMouseButtonState(button);
}

bool Input::isMouseButtonReleased(MouseButton button) const
{
    return !getMouseButtonState(button) && getPrevMouseButtonState(button);
}

void Input::getMousePos(int* x, int* y) const
{
    *x = static_cast<int>(_mouse_pos.x);
    *y = static_cast<int>(_mouse_pos.y);
}

void Input::getMouseDelta(int* x, int* y) const
{
    *x = static_cast<int>(_mouse_delta.x);
    *y = static_cast<int>(_mouse_delta.y);
}

bool Input::isMouseCaptured() const
{
    return _mouse_captured;
}

void Input::setMouseCaptured(bool captured)
{
    _mouse_captured = captured;

    // if (mouse_captured) {
    //     SDL_GetMouseState(&last_mouse_pos.x, &last_mouse_pos.y);
    // } else {
    //     SDL_WarpMouseInWindow(window.getWindow(), last_mouse_pos.x, last_mouse_pos.y);
    // }

    SDL_SetWindowRelativeMouseMode(g_Window->getHandle(), _mouse_captured);
    SDL_GetRelativeMouseState(nullptr, nullptr);
}

bool Input::getKeyState(Key key) const
{
    auto it = _keys.find(key);
    return it != _keys.end() ? it->second : false;
}

bool Input::getPrevKeyState(Key key) const
{
    auto it = _prev_keys.find(key);
    return it != _prev_keys.end() ? it->second : false;
}

bool Input::getMouseButtonState(MouseButton button) const
{
    return _mouse_buttons.test(static_cast<size_t>(button));
}

bool Input::getPrevMouseButtonState(MouseButton button) const
{
    return _prev_mouse_buttons.test(static_cast<size_t>(button));
}
