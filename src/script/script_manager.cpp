#include "script/script_manager.h"

#include "core/console.h"
#include "core/globals.h"
#include "vfs/vfs.h"

#include <sstream>

bool ScriptManager::execCon(const std::string& path)
{
    auto content = g_VFS->readFileString(path);
    if (content.empty())
    {
        LOG_ERROR("Console::execFile: Failed to read content from '%s'!", path.c_str());
        return false;
    }

    // std::string full_path = g_VFS->findFile(path);
    // LOG_DEBUG("ScriptManager::execCon: Executing file '%s'...", full_path.c_str());

    Console::ExecContext ctx;

    std::istringstream stream(content);
    std::string line;
    int line_number = 0;

    while (std::getline(stream, line))
    {
        line_number++;

        std::erase_if(line, [](char c) {
            return c == '\t' || c == '\r' || c == '\"' || c == '\'' || c == ';';
        });

        if (!g_Console->exec(line, ctx))
        {
            LOG_ERROR("Console::execFile: Error at line %d in '%s'!", line_number, path.c_str());
            return false;
        }
    }

    return true;
}

std::future<bool> ScriptManager::execConAsync(const std::string& path)
{
    return _pool.enqueue([this, path] { return execCon(path); });
}