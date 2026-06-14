#include "core/console.h"
#include "core/engine.h"
#include "game//game.h"
#include "render/renderer.h"
#include "render/texture.h"
#include "geometry/template.h"
#include "object/template.h"
#include "object/object.h"
#include "vfs/vfs.h"
#include "utils/string_utils.h"
#include "utils/log.h"
#include "world/sky.h"
#include "world/water.h"

#include <sstream>

#define REGISTER_OBJECT_PROPERTY(ObjectName, ObjectRef, PropertyName, SetterFunction) \
    registerCmd(std::string(#ObjectName) + "." + #PropertyName, [&engine](const CommandArgs& args) \
    { \
        if (args.empty()) \
        { \
            LOG_ERROR("Console: %s.%s: Not enough arguments!", #ObjectName, #PropertyName); \
            return false; \
        } \
        std::string value_str = Console::joinArgs(args); \
        auto* obj_ptr = (ObjectRef); \
        SetterFunction(obj_ptr, value_str); \
        return true; \
    })

#define GEN_STRING_SETTER(Class, Field) \
    [](Class* obj, const std::string& value) { \
        if (obj) obj->Field = Console::parseString(value); \
    }
    
#define GEN_INT_SETTER(Class, Field) \
    [](Class* obj, const std::string& value) { \
        if (obj) obj->Field = Console::parseInt(value); \
    }
    
#define GEN_FLOAT_SETTER(Class, Field) \
    [](Class* obj, const std::string& value) { \
        if (obj) obj->Field = Console::parseFloat(value); \
    }

#define GEN_VEC2_SETTER(Class, Field) \
    [](Class* obj, const std::string& value) { \
        if (obj) obj->Field = Console::parseVec2(value); \
    }

#define GEN_VEC3_SETTER(Class, Field) \
    [](Class* obj, const std::string& value) { \
        if (obj) obj->Field = Console::parseVec3(value); \
    }

void Console::init(const Engine& engine)
{
    LOG_INFO("Console::init: Initializing console...");

    registerCmd("rem", [](const CommandArgs& args) { return true; });
    registerCmd("REM", [](const CommandArgs& args) { return true; });

    registerCmd("run", [&engine](const CommandArgs& args)
    {
        if (args.empty()) {
            LOG_ERROR("Console: run: Not enough arguments!");
            return false;
        }
        std::string path = args[0];
        if (!path.ends_with(".con")) path += ".con";

        return engine.getConsole().execFile(path);
    });

    registerCmd("GeometryTemplate.create", [](const CommandArgs& args)
    {
        if (args.size() < 2)
        {
            LOG_ERROR("Console: GeometryTemplate.create: Not enough arguments!");
            return false;
        }

        GeometryType type = geometryTypeFromString(args[0]);

        if (type == GeometryType::Unknown)
        {
            LOG_ERROR("Console: GeometryTemplate.create: Unknown geometry type '%s'!", args[0].c_str());
            return false;
        }

        GeometryTemplate::create(args[1], type);

        return true;
    });

    registerCmd("ObjectTemplate.create", [](const CommandArgs& args)
    {
        if (args.size() < 2)
        {
            LOG_ERROR("Console: ObjectTemplate.create: Not enough arguments!");
            return false;
        }

        ObjectType type = objectTypeFromString(args[0]);

        if (type == ObjectType::Unknown)
        {
            LOG_WARNING("Console: ObjectTemplate.create: Unknown object type '%s'!", args[0].c_str());
            return true;
        }
        
        ObjectTemplate::create(args[1], type);

        return true;
    });

    registerCmd("ObjectTemplate.addTemplate", [](const CommandArgs& args)
    {
        if (args.empty())
        {
            LOG_ERROR("Console: ObjectTemplate.addTemplate: Not enough arguments!");
            return false;
        }

        auto current = ObjectTemplate::current;
        if (current)
        {
            ObjectTemplate::last_added_child = &current->children.emplace_back(
                args[0], glm::vec3(0.0f), glm::vec3(0.0f)
            );
        }

        return true;
    });

    registerCmd("ObjectTemplate.setPosition", [](const CommandArgs& args)
    {
        if (args.empty())
        {
            LOG_ERROR("Console: ObjectTemplate.setPosition: Not enough arguments!");
            return false;
        }

        auto current = ObjectTemplate::current;
        if (current && current->last_added_child)
            current->last_added_child->position = Console::parseVec3(args[0]);

        return true;
    });

    registerCmd("ObjectTemplate.setRotation", [](const CommandArgs& args)
    {
        if (args.empty())
        {
            LOG_ERROR("Console: ObjectTemplate.setRotation: Not enough arguments!");
            return false;
        }

        auto current = ObjectTemplate::current;
        if (current && current->last_added_child)
            current->last_added_child->rotation = Console::parseVec3(args[0]);

        return true;
    });

    registerCmd("Object.create", [&engine](const CommandArgs& args)
    {
        Object::current = nullptr;
        
        if (args.empty())
        {
            LOG_ERROR("Console: Object.create: Not enough arguments!");
            return false;
        }

        ObjectTemplate* tmpl = ObjectTemplate::get(args[0]);
        if (!tmpl)
        {
            LOG_ERROR("Console: Object.create: Object template with name '%s' not found!", args[0].c_str());
            return true;
        }

        auto& world = engine.getWorld();
        if (!(Object::current = world.createObject(tmpl)))
        {
            // PatchTerrain initializes level-wide terrain globally and never creates an Object (always returns nullptr).
            GeometryTemplate* geom_tmpl = GeometryTemplate::get(tmpl->geometry);
            if (geom_tmpl && geom_tmpl->type == GeometryType::PatchTerrain)
                return true;
            
            LOG_ERROR("Console: Object.create: Failed to create object!");
            return false;
        }

        return true;
    });

    registerCmd("Sky.initSky", [&engine](const CommandArgs& args)
    {
        return engine.getWorld().getSky().init();
    });

    // GeometryTemplate
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, file, GEN_STRING_SETTER(GeometryTemplate, file));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, materialMap, GEN_STRING_SETTER(GeometryTemplate, material_map));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, texBaseName, GEN_STRING_SETTER(GeometryTemplate, tex_base_name));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, detailTexName, GEN_STRING_SETTER(GeometryTemplate, detail_tex_name));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, materialSize, GEN_INT_SETTER(GeometryTemplate, material_size));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, worldSize, GEN_INT_SETTER(GeometryTemplate, world_size));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, texOffsetX, GEN_INT_SETTER(GeometryTemplate, tex_offset_x));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, texOffsetY, GEN_INT_SETTER(GeometryTemplate, tex_offset_y));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, waterLevel, GEN_INT_SETTER(GeometryTemplate, water_level));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, GeometryTemplate::current, yScale, GEN_FLOAT_SETTER(GeometryTemplate, y_scale));

    // ObjectTemplate
    REGISTER_OBJECT_PROPERTY(ObjectTemplate, ObjectTemplate::current, geometry, GEN_STRING_SETTER(ObjectTemplate, geometry));
    REGISTER_OBJECT_PROPERTY(ObjectTemplate, ObjectTemplate::current, setContinousRotationSpeed, GEN_VEC3_SETTER(ObjectTemplate, continous_rot_speed));

    // Object
    REGISTER_OBJECT_PROPERTY(Object, Object::current, absolutePosition, [](Object* obj, const std::string& value) {
        if (obj) obj->setPosition(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Object, Object::current, rotation, [](Object* obj, const std::string& value) {
        if (obj) obj->setRotation(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Object, Object::current, geometry.scale, [](Object* obj, const std::string& value) {
        if (obj) obj->setScale(Console::parseVec3(value));
    });

    // Renderer
    REGISTER_OBJECT_PROPERTY(Renderer, &engine.getRenderer(), fogColorVec, [](Renderer* r, const std::string& value) {
        if (r) r->setFogColor(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, &engine.getRenderer(), fogStart, [](Renderer* r, const std::string& value) {
        if (r) r->setFogStart(Console::parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, &engine.getRenderer(), fogEnd, [](Renderer* r, const std::string& value) {
        if (r) r->setFogEnd(Console::parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, &engine.getRenderer(), vertexFogEnable, [](Renderer* r, const std::string& value) {
        if (r) r->setFogEnabled(Console::parseInt(value) != 0);
    });

    REGISTER_OBJECT_PROPERTY(Renderer, &engine.getRenderer(), diffuseColor, [](Renderer* r, const std::string& value) {
        if (r) r->setDiffuseLight(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, &engine.getRenderer(), specularColor, [](Renderer* r, const std::string& value) {
        if (r) r->setSpecularLight(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, &engine.getRenderer(), ambientColor, [](Renderer* r, const std::string& value) {
        if (r) r->setAmbientLight(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, &engine.getRenderer(), globalAmbientColor, [](Renderer* r, const std::string& value) {
        if (r) r->setGlobalAmbientLight(Console::parseVec3(value));
    });

    // Game
    REGISTER_OBJECT_PROPERTY(Game, &engine.getGame(), setViewDistance, [](Game* g, const std::string& value) {
        if (g) g->setViewDistance(Console::parseFloat(value));
    });

    // Sky
    REGISTER_OBJECT_PROPERTY(Sky, &engine.getRenderer(), sunLightDirectionVec, [](Renderer* r, const std::string& value) {
        if (r) r->setSunDirection(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Sky, &engine.getWorld().getSky(), setRotAngle, GEN_FLOAT_SETTER(Sky, rot_angle));

    // Water
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), texLayer1, [](Water* w, const std::string& value) {
        w->setTexture(0, Texture::load(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), texLayer2, [](Water* w, const std::string& value) {
        w->setTexture(1, Texture::load(value));
    });
    
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), scrollDirection1, [](Water* w, const std::string& value) {
        w->setScrollDir(0, parseVec2(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), scrollDirection2, [](Water* w, const std::string& value) {
        w->setScrollDir(1, parseVec2(value));
    });

    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), scrollLayer1, [](Water* w, const std::string& value) {
        w->setScrollSpeed(0, parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), scrollLayer2, [](Water* w, const std::string& value) {
        w->setScrollSpeed(1, parseFloat(value));
    });

    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), tileLayer1, [](Water* w, const std::string& value) {
        w->setUVScale(0, parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), tileLayer2, [](Water* w, const std::string& value) {
        w->setUVScale(1, parseFloat(value));
    });

    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), color, [](Water* w, const std::string& value) {
        w->setColor(parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), deepColor, [](Water* w, const std::string& value) {
        w->setDeepColor(parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), waterColorDepth, [](Water* w, const std::string& value) {
        w->setColorDepth(parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), waterAlphaDepth, [](Water* w, const std::string& value) {
        w->setAlphaDepth(parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &engine.getWorld().getWater(), waterShallowAlpha, [](Water* w, const std::string& value) {
        w->setShallowAlpha(parseFloat(value));
    });
}

void Console::registerCmd(const std::string& name, CommandHandler fn)
{
    _commands[StringUtils::lowercase(name)] = fn;
}

bool Console::exec(const std::string& line)
{
    if (line.empty()) return true;

    auto tokens = StringUtils::split(line);
    if (tokens.empty()) return true;
    
    std::string cmd = StringUtils::lowercase(tokens[0]);
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    auto it = _commands.find(cmd);
    if (it != _commands.end())
    {
        if (!it->second(args))
            return true;
        return true;
    }
    else
    {
        // LOG_WARNING("Console::exec: Unknown command: %s", cmd.c_str());
        return true;
    }
}

bool Console::execFile(const std::string& path)
{
    auto content = VFS::readFileString(path);
    if (content.empty())
    {
        LOG_ERROR("Console::execFile: Failed to read content from '%s'!", path.c_str());
        return false;
    }

    // std::string full_path = VFS::findFile(path);
    // LOG_DEBUG("Console::execFile: Executing file '%s'...", full_path.c_str());

    std::istringstream stream(content);
    std::string line;
    int line_number = 0;

    while (std::getline(stream, line))
    {
        line_number++;

        std::erase_if(line, [](char c) {
            return c == '\t' || c == '\r' || c == '\"' || c == '\'' || c == ';';
        });

        if (!exec(line))
        {
            LOG_ERROR("Console::execFile: Error at line %d in '%s'!", line_number, path.c_str());
            return false;
        }
    }

    return true;
}

int Console::parseInt(const std::string& str)
{
    try { return std::stoi(str); } catch (...) { return 0; }
}

float Console::parseFloat(const std::string& str)
{
    try { return std::stof(str); } catch (...) { return 0.0f; }
}

std::string Console::parseString(const std::string& str)
{
    return str;
}

glm::vec2 Console::parseVec2(const std::string& str)
{
    auto vec_strs = StringUtils::split(str, '/');
    if (vec_strs.size() < 2) return glm::vec2(0.0f);

    return glm::vec2(
        parseFloat(vec_strs[0]),
        parseFloat(vec_strs[1])
    );
}

glm::vec3 Console::parseVec3(const std::string& str)
{
    auto vec_strs = StringUtils::split(str, '/');
    if (vec_strs.size() < 3) return glm::vec3(0.0f);

    return glm::vec3(
        parseFloat(vec_strs[0]),
        parseFloat(vec_strs[1]),
        parseFloat(vec_strs[2])
    );
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
