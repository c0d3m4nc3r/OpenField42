#include "ui/console_ui.h"

#include "core/console.h"
#include "core/globals.h"

#include <cstring>
#include <format>

ConsoleUI::ConsoleUI()
{
    std::memset(_input_buffer, 0, sizeof(_input_buffer));
}

void ConsoleUI::toggle()
{
    _is_open = !_is_open;
    if (_is_open) _reclaim_focus = true;
}

void ConsoleUI::render()
{
    if (!_is_open) return;

    ImGuiIO& io = ImGui::GetIO();
    float consoleHeight = io.DisplaySize.y * 0.35f;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, consoleHeight));
    ImGui::SetNextWindowBgAlpha(0.85f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    if (ImGui::Begin("Console", nullptr, flags))
    {
        const float footer_height_to_reserve = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
        if (ImGui::BeginChild("ConsoleLogScroll", ImVec2(0.0f, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar))
        { 
            for (const auto& log : _logs)
            {
                ImGui::TextUnformatted(log.c_str());
            }

            if (_scroll_to_bottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }
            _scroll_to_bottom = false;
        }
        ImGui::EndChild();

        ImGui::Separator();

        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue |
                                         ImGuiInputTextFlags_CallbackHistory |
                                         ImGuiInputTextFlags_CallbackCompletion;

        ImGui::PushItemWidth(-1.0f);
        
        if (_reclaim_focus)
        {
            ImGui::SetKeyboardFocusHere();
            _reclaim_focus = false;
        }

        if (ImGui::InputText("##ConsoleInput", _input_buffer, IM_ARRAYSIZE(_input_buffer), input_flags, &ConsoleUI::textEditCallbackStub, this))
        {
            executeCommand();
        }

        ImGui::PopItemWidth();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);
}

void ConsoleUI::executeCommand()
{
    std::string command(_input_buffer);

    _input_buffer[0] = '\0';
    _reclaim_focus = true;

    if (command.empty()) return;

    auto result = g_Console->exec(command);

    if (!result.empty()) {
        _logs.push_back(std::format("> {}\n{}", command, result.message));
    } else {
        _logs.push_back(std::format("> {}", command));
    }

    _history.push_back(std::move(command));
    _history_pos = -1;

    _scroll_to_bottom = true;
}

int ConsoleUI::textEditCallbackStub(ImGuiInputTextCallbackData* data)
{
    auto* console = static_cast<ConsoleUI*>(data->UserData);
    return console->textEditCallback(data);
}

int ConsoleUI::textEditCallback(ImGuiInputTextCallbackData* data)
{
    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackHistory:
    {
        const int prev_history_pos = _history_pos;
        if (data->EventKey == ImGuiKey_UpArrow)
        {
            if (_history_pos == -1)
                _history_pos = static_cast<int>(_history.size()) - 1;
            else if (_history_pos > 0)
                _history_pos--;
        }
        else if (data->EventKey == ImGuiKey_DownArrow)
        {
            if (_history_pos != -1)
            {
                _history_pos++;
                if (_history_pos >= static_cast<int>(_history.size()))
                    _history_pos = -1;
            }
        }

        if (prev_history_pos != _history_pos)
        {
            const char* history_str = (_history_pos >= 0) ? _history[_history_pos].c_str() : "";
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, history_str);
        }
        break;
    }
    case ImGuiInputTextFlags_CallbackCompletion:
    {
        std::string_view current_input(data->Buf, static_cast<size_t>(data->BufTextLen));
        auto completions = g_Console->getCompletions(current_input);

        if (completions.empty())
        {
            break;
        }

        if (completions.size() == 1)
        {
            data->DeleteChars(0, data->BufTextLen);
            
            data->InsertChars(data->CursorPos, completions[0].data(), completions[0].data() + completions[0].size());
            data->InsertChars(data->BufTextLen, " ");
        }
        else
        {
            std::string log_msg = std::format("> {}\n  Matches:\n", current_input);
            for (const auto& match : completions)
            {
                log_msg += std::format("    {}\n", match);
            }
            _logs.push_back(std::move(log_msg));
            _scroll_to_bottom = true;

            // 4. Дополняем до общего префикса
            std::string common = g_Console->autocomplete(current_input);
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, common.c_str());
        }

        break;
    }
    }
    return 0;
}