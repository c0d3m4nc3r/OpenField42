#pragma once

#include "core/thread_pool.h"

class ScriptManager
{
public:

    bool execCon(const std::string& path);
    std::future<bool> execConAsync(const std::string& path);

private:

    ThreadPool _pool{4};

};
