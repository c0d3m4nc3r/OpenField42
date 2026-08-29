#pragma once

#include "object/object_template.h"

#include <string>
#include <functional>

#include <glm/fwd.hpp>

struct GeometryTemplate;
struct Object;

class Engine;
class Console
{
public:

    struct ExecContext
    {
        GeometryTemplate* last_geom_tmpl = nullptr;
        ObjectTemplate* last_obj_tmpl = nullptr;
        ObjectTemplate::Child* last_child = nullptr;
        Object* last_obj = nullptr;
    };

    using CommandArgs = std::vector<std::string>;
    using CommandHandler = std::function<bool(ExecContext&, const CommandArgs&)>;

    void init();

    void registerCmd(const std::string& name, CommandHandler handler);

    bool exec(const std::string& line, ExecContext& ctx);
    bool exec(const std::string& line)
    {
        return exec(line, _main_exec_ctx);
    }

    static int parseInt(const std::string& str);
    static float parseFloat(const std::string& str);
    static std::string parseString(const std::string& str);
    static glm::vec2 parseVec2(const std::string& str);
    static glm::vec3 parseVec3(const std::string& str);

    static std::string joinArgs(const CommandArgs& args);

private:

    std::unordered_map<std::string, CommandHandler> _commands;
    ExecContext _main_exec_ctx;

};
