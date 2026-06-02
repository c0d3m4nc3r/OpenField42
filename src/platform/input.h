#pragma once

#include "platform/input_enums.h"

#include <SDL3/SDL_events.h>

#include <glm/vec2.hpp>

#include <unordered_map>
#include <bitset>

class Input
{
public:

    void onEvent(const SDL_Event& event);
    void update();

    bool isKeyUp(Key key) const;
    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;
    bool isKeyReleased(Key key) const;

    bool isMouseButtonUp(MouseButton button) const;
    bool isMouseButtonDown(MouseButton button) const;
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonReleased(MouseButton button) const;

    void getMousePos(int* x, int* y) const;
    void getMouseDelta(int* x, int* y) const;

    bool isMouseCaptured() const;
    void setMouseCaptured(bool captured);

private:

    std::unordered_map<Key, bool> keys;
    std::unordered_map<Key, bool> prev_keys;

    std::bitset<8> mouse_buttons;
    std::bitset<8> prev_mouse_buttons;

    glm::vec2 mouse_pos, mouse_delta;

    bool mouse_captured = false;
    
    bool getKeyState(Key key) const;
    bool getPrevKeyState(Key key) const;
    bool getMouseButtonState(MouseButton button) const;
    bool getPrevMouseButtonState(MouseButton button) const;
};

extern Input g_Input;