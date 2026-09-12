#include "script/script_manager.h"

#include "core/console.h"
#include "core/globals.h"
#include "vfs/vfs.h"

#include <sstream>

bool ScriptManager::execCon(std::string_view path)
{
    auto content = g_VFS->readFileString(path);
    if (content.empty())
    {
        LOG_ERROR("ScriptManager::execCon: Failed to read content from '%.*s'!",
            static_cast<int>(path.size()), path.data());
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

        auto result = g_Console->exec(line, ctx);

        switch (result.status)
        {
        case CommandStatus::Error:
            LOG_ERROR("ScriptManager::execCon: Error at line %d in '%.*s'!",
                line_number, static_cast<int>(path.size()), path.data());
            LOG_ERROR("ScriptManager::execCon: %s", result.message.c_str());
            return false;
        case CommandStatus::Warning:
            LOG_WARNING("ScriptManager::execCon: Warning at line %d in '%.*s'!",
                line_number, static_cast<int>(path.size()), path.data());
            LOG_WARNING("ScriptManager::execCon: %s", result.message.c_str());
            break;
        default:
            break;
        }
    }

    return true;
}

std::future<bool> ScriptManager::execConAsync(std::string_view path)
{
    return g_ThreadPool.enqueue([this, path] { return execCon(path); });
}