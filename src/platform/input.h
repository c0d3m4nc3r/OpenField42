#pragma once

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_events.h>

#include <glm/vec2.hpp>

#include <unordered_map>
#include <bitset>

enum class MouseButton;
enum class Key;

class Input
{
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

};

enum class MouseButton
{
    Left = SDL_BUTTON_LEFT,
    Middle = SDL_BUTTON_MIDDLE,
    Right = SDL_BUTTON_RIGHT,
    X1 = SDL_BUTTON_X1,
    X2 = SDL_BUTTON_X2,
};

enum class Key
{
    Unknown = SDL_SCANCODE_UNKNOWN,
    A = SDL_SCANCODE_A, B = SDL_SCANCODE_B, C = SDL_SCANCODE_C,
    D = SDL_SCANCODE_D, E = SDL_SCANCODE_E, F = SDL_SCANCODE_F,
    G = SDL_SCANCODE_G, H = SDL_SCANCODE_H, I = SDL_SCANCODE_I,
    J = SDL_SCANCODE_J, K = SDL_SCANCODE_K, L = SDL_SCANCODE_L,
    M = SDL_SCANCODE_M, N = SDL_SCANCODE_N, O = SDL_SCANCODE_O,
    P = SDL_SCANCODE_P, Q = SDL_SCANCODE_Q, R = SDL_SCANCODE_R,
    S = SDL_SCANCODE_S, T = SDL_SCANCODE_T, U = SDL_SCANCODE_U,
    V = SDL_SCANCODE_V, W = SDL_SCANCODE_W, X = SDL_SCANCODE_X,
    Y = SDL_SCANCODE_Y, Z = SDL_SCANCODE_Z,

    Num0 = SDL_SCANCODE_0, Num1 = SDL_SCANCODE_1, Num2 = SDL_SCANCODE_2,
    Num3 = SDL_SCANCODE_3, Num4 = SDL_SCANCODE_4, Num5 = SDL_SCANCODE_5,
    Num6 = SDL_SCANCODE_6, Num7 = SDL_SCANCODE_7, Num8 = SDL_SCANCODE_8,
    Num9 = SDL_SCANCODE_9,

    F1  = SDL_SCANCODE_F1,  F2  = SDL_SCANCODE_F2,  F3  = SDL_SCANCODE_F3,
    F4  = SDL_SCANCODE_F4,  F5  = SDL_SCANCODE_F5,  F6  = SDL_SCANCODE_F6,
    F7  = SDL_SCANCODE_F7,  F8  = SDL_SCANCODE_F8,  F9  = SDL_SCANCODE_F9,
    F10 = SDL_SCANCODE_F10, F11 = SDL_SCANCODE_F11, F12 = SDL_SCANCODE_F12,

    LShift = SDL_SCANCODE_LSHIFT, RShift = SDL_SCANCODE_RSHIFT,
    LCtrl  = SDL_SCANCODE_LCTRL,  RCtrl  = SDL_SCANCODE_RCTRL,
    LAlt   = SDL_SCANCODE_LALT,   RAlt   = SDL_SCANCODE_RALT,
    Space  = SDL_SCANCODE_SPACE,
    Enter  = SDL_SCANCODE_RETURN,

    Up     = SDL_SCANCODE_UP,
    Down   = SDL_SCANCODE_DOWN,
    Left   = SDL_SCANCODE_LEFT,
    Right  = SDL_SCANCODE_RIGHT,

    Home   = SDL_SCANCODE_HOME,
    End    = SDL_SCANCODE_END,
    PageUp = SDL_SCANCODE_PAGEUP,
    PageDown = SDL_SCANCODE_PAGEDOWN,

    Insert = SDL_SCANCODE_INSERT,
    Delete = SDL_SCANCODE_DELETE,

    Escape = SDL_SCANCODE_ESCAPE,
    Tab    = SDL_SCANCODE_TAB,
    Backspace = SDL_SCANCODE_BACKSPACE,

    Numpad0 = SDL_SCANCODE_KP_0, Numpad1 = SDL_SCANCODE_KP_1, Numpad2 = SDL_SCANCODE_KP_2,
    Numpad3 = SDL_SCANCODE_KP_3, Numpad4 = SDL_SCANCODE_KP_4, Numpad5 = SDL_SCANCODE_KP_5,
    Numpad6 = SDL_SCANCODE_KP_6, Numpad7 = SDL_SCANCODE_KP_7, Numpad8 = SDL_SCANCODE_KP_8,
    Numpad9 = SDL_SCANCODE_KP_9, NumpadEnter = SDL_SCANCODE_KP_ENTER,
    NumpadPlus = SDL_SCANCODE_KP_PLUS, NumpadMinus = SDL_SCANCODE_KP_MINUS,
    NumpadMultiply = SDL_SCANCODE_KP_MULTIPLY, NumpadDivide = SDL_SCANCODE_KP_DIVIDE,

    NumpadPeriod = SDL_SCANCODE_KP_PERIOD,
    NumpadEquals = SDL_SCANCODE_KP_EQUALS,

    PrintScreen = SDL_SCANCODE_PRINTSCREEN,
    ScrollLock  = SDL_SCANCODE_SCROLLLOCK,
    Pause       = SDL_SCANCODE_PAUSE
};

extern Input input;