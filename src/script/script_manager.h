#pragma once

#include <future>

class ScriptManager
{
public:

    bool execCon(std::string_view path);
    std::future<bool> execConAsync(std::string_view path);

};
