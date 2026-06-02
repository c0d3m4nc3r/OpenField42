#pragma once

#include <string>
#include <functional>

#include <glm/fwd.hpp>

class Console
{
public:

    using CommandArgs = std::vector<std::string>;
    using CommandHandler = std::function<bool(const CommandArgs&)>;

    void init();

    void registerCmd(const std::string& name, CommandHandler handler);

    bool exec(const std::string& line);
    bool execFile(const std::string& path);

    static int parseInt(const std::string& str);
    static float parseFloat(const std::string& str);
    static std::string parseString(const std::string& str);
    static glm::vec2 parseVec2(const std::string& str);
    static glm::vec3 parseVec3(const std::string& str);

    static std::string joinArgs(const CommandArgs& args);

private:

    std::unordered_map<std::string, CommandHandler> commands;

};

extern Console g_Console;