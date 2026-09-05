#pragma once

#include "imgui.h"

class ConsoleUI
{
public:

    ConsoleUI();

    void render();

    void toggle();

    bool isOpen() const { return _is_open; }

private:

    std::vector<std::string> _logs;

    char _input_buffer[256] = {};
    std::vector<std::string> _history;
    int _history_pos = -1;

    bool _is_open = false;
    bool _scroll_to_bottom = false;
    bool _reclaim_focus = false;

    void executeCommand();
    static int textEditCallbackStub(ImGuiInputTextCallbackData* data);
    int textEditCallback(ImGuiInputTextCallbackData* data);

};
