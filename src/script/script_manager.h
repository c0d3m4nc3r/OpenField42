#pragma once

#include <future>

class ScriptManager
{
public:

    bool execCon(const std::string& path);
    std::future<bool> execConAsync(const std::string& path);

};
