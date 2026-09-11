#include "core/console.h"

#include "core/globals.h"
#include "script/script_manager.h"
#include "utils/string_utils.h"
#include "utils/log.h"

#include <algorithm>

void Console::init()
{
    LOG_INFO("Console::init: Initializing console...");

    registerCmd("sex", [](ExecContext& ctx, const CommandArgs& args) -> CommandResult { return { "No sex :(", CommandStatus::Error }; });

    registerCmd("rem", [](ExecContext& ctx, const CommandArgs& args) -> CommandResult { return {}; });
    registerCmd("REM", [](ExecContext& ctx, const CommandArgs& args) -> CommandResult { return {}; });

    registerCmd("run", [](ExecContext& ctx, const CommandArgs& args) -> CommandResult
    {
        if (args.empty())
        {
            LOG_ERROR("Console: run: Not enough arguments!");
            return CommandResult{ "Not enough arguments! Usage: run <script.con>", CommandStatus::Error };
        }
        std::string path = args[0];
        if (!path.ends_with(".con")) path += ".con";

        bool status = g_ScriptMgr->execCon(path);
        if (!status)
        {
            return CommandResult{ "Failed to execute script: " + path, CommandStatus::Error };
        }

        return CommandResult{ "Executed script: " + path, CommandStatus::Success };
    });
}

void Console::registerCmd(const std::string& name, CommandHandler fn)
{
    _commands[StringUtils::lowercase(name)] = std::move(fn);
}

void Console::addAlias(const std::string& alias, const std::string& command)
{
    _aliases[StringUtils::lowercase(alias)] = StringUtils::lowercase(command);
}

CommandResult Console::exec(const std::string& line, ExecContext& ctx)
{
    if (line.empty()) return CommandResult{ "Command is empty!", CommandStatus::Warning };

    auto tokens = StringUtils::split(line);
    if (tokens.empty()) return CommandResult{ "Command is empty!", CommandStatus::Warning };
    
    std::string cmd = StringUtils::lowercase(tokens[0]);
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    auto alias_it = _aliases.find(cmd);
    if (alias_it != _aliases.end())
    {
        cmd = alias_it->second;
    }

    auto it = _commands.find(cmd);
    if (it != _commands.end())
    {
        return it->second(ctx, args);
    }
    else
    {
        // LOG_WARNING("Console::exec: Unknown command: %s", cmd.c_str());
        return CommandResult { "Unknown command!", CommandStatus::Warning };
    }
}

std::string Console::joinArgs(const CommandArgs& args)
{
    if (args.empty()) return "";
    if (args.size() == 1) return args[0];
    
    std::string result;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) result += " ";
        result += args[i];
    }
    return result;
}

std::vector<std::string> Console::getCompletions(std::string_view prefix) const
{
    std::string lower_prefix = StringUtils::lowercase(std::string(prefix));
    std::vector<std::string> matches;

    for (const auto& [name, handler] : _commands)
    {
        if (name.rfind(lower_prefix, 0) == 0) 
        {
            matches.push_back(name);
        }
    }

    for (const auto& [name, _] : _aliases)
    {
        if (name.rfind(lower_prefix, 0) == 0) 
        {
            matches.push_back(name);
        }
    }

    std::sort(matches.begin(), matches.end());
    return matches;
}

std::string Console::autocomplete(std::string_view input) const
{
    auto matches = getCompletions(input);

    if (matches.empty())
    {
        return std::string(input);
    }

    if (matches.size() == 1)
    {
        return matches[0] + " ";
    }

    std::string common_prefix = matches[0];
    for (size_t i = 1; i < matches.size(); ++i)
    {
        size_t j = 0;
        while (j < common_prefix.size() && j < matches[i].size() && common_prefix[j] == matches[i][j])
        {
            ++j;
        }
        common_prefix.resize(j);
    }

    return common_prefix;
}