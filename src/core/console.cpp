#include "core/console.h"

#include "core/engine.h"
#include "core/globals.h"
#include "game//game.h"
#include "geometry/geometry_manager.h"
#include "geometry/geometry_template.h"
#include "object/object_manager.h"
#include "object/object_template.h"
#include "object/object.h"
#include "render/renderer.h"
#include "script/script_manager.h"
#include "utils/string_utils.h"
#include "utils/log.h"
#include "world/sky.h"
#include "world/water.h"
#include "world/world.h"

#define REGISTER_OBJECT_PROPERTY(ObjectName, ObjectRef, PropertyName, SetterFunction) \
    registerCmd(std::string(#ObjectName) + "." + #PropertyName, [&](const CommandArgs& args) \
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

void Console::init()
{
    LOG_INFO("Console::init: Initializing console...");

    registerCmd("rem", [](const CommandArgs& args) { return true; });
    registerCmd("REM", [](const CommandArgs& args) { return true; });

    registerCmd("run", [](const CommandArgs& args)
    {
        if (args.empty())
        {
            LOG_ERROR("Console: run: Not enough arguments!");
            return false;
        }
        std::string path = args[0];
        if (!path.ends_with(".con")) path += ".con";

        return g_ScriptMgr->execCon(path);
    });

    registerCmd("GeometryTemplate.create", [this](const CommandArgs& args)
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

        _current_geom_tmpl = g_GeometryMgr->createTemplate(args[1], type);

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
        
        ObjectTemplate::current = &g_ObjectMgr->createTemplate(args[1], type);

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

    registerCmd("Object.create", [](const CommandArgs& args)
    {
        Object::current = nullptr;
        
        if (args.empty())
        {
            LOG_ERROR("Console: Object.create: Not enough arguments!");
            return false;
        }

        ObjectTemplate* tmpl = g_ObjectMgr->getTemplate(args[0]);
        if (!tmpl)
        {
            LOG_ERROR("Console: Object.create: Object template with name '%s' not found!", args[0].c_str());
            return true;
        }

        if (!(Object::current = g_World->createObject(tmpl)))
        {
            // PatchTerrain initializes level-wide terrain globally and never creates an Object (always returns nullptr).
            GeometryTemplate* geom_tmpl = g_GeometryMgr->getTemplate(tmpl->geometry);
            if (geom_tmpl && geom_tmpl->type == GeometryType::PatchTerrain)
                return true;
            
            LOG_ERROR("Console: Object.create: Failed to create object!");
            return false;
        }

        return true;
    });

    registerCmd("Sky.initSky", [this](const CommandArgs& args)
    {
        return g_World->getSky().init(_current_geom_tmpl);
    });

    // GeometryTemplate
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, file, GEN_STRING_SETTER(GeometryTemplate, file));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, materialMap, GEN_STRING_SETTER(GeometryTemplate, material_map));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, texBaseName, GEN_STRING_SETTER(GeometryTemplate, tex_base_name));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, detailTexName, GEN_STRING_SETTER(GeometryTemplate, detail_tex_name));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, materialSize, GEN_INT_SETTER(GeometryTemplate, material_size));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, worldSize, GEN_INT_SETTER(GeometryTemplate, world_size));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, texOffsetX, GEN_INT_SETTER(GeometryTemplate, tex_offset_x));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, texOffsetY, GEN_INT_SETTER(GeometryTemplate, tex_offset_y));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, waterLevel, GEN_INT_SETTER(GeometryTemplate, water_level));
    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, yScale, GEN_FLOAT_SETTER(GeometryTemplate, y_scale));

    REGISTER_OBJECT_PROPERTY(GeometryTemplate, _current_geom_tmpl, setLodDistance, [](GeometryTemplate* tmpl, const std::string& value) {
        auto args = StringUtils::split(value);
        if (args.size() < 2)
        {
            LOG_ERROR("Console: GeometryTemplate.setLodDistance: Not enough arguments!");
            return;
        }

        int level = parseInt(args[0]);
        float distance = parseFloat(args[1]);

        tmpl->lod_distances[level] = distance;
    });

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
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, fogColorVec, [](Renderer* r, const std::string& value) {
        if (r) r->setFogColor(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, fogStart, [](Renderer* r, const std::string& value) {
        if (r) r->setFogStart(Console::parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, fogEnd, [](Renderer* r, const std::string& value) {
        if (r) r->setFogEnd(Console::parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, fogLinearStart, [](Renderer* r, const std::string& value) {
        if (r) r->setFogStart(Console::parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, fogLinearEnd, [](Renderer* r, const std::string& value) {
        if (r) r->setFogEnd(Console::parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, vertexFogEnable, [](Renderer* r, const std::string& value) {
        if (r) r->setFogEnabled(Console::parseInt(value) != 0);
    });

    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, diffuseColor, [](Renderer* r, const std::string& value) {
        if (r) r->setDiffuseLight(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, specularColor, [](Renderer* r, const std::string& value) {
        if (r) r->setSpecularLight(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, ambientColor, [](Renderer* r, const std::string& value) {
        if (r) r->setAmbientLight(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Renderer, g_Renderer, globalAmbientColor, [](Renderer* r, const std::string& value) {
        if (r) r->setGlobalAmbientLight(Console::parseVec3(value));
    });

    // Game
    REGISTER_OBJECT_PROPERTY(Game, g_Game, setViewDistance, [](Game* g, const std::string& value) {
        if (g) g->setViewDistance(Console::parseFloat(value));
    });

    // Sky
    REGISTER_OBJECT_PROPERTY(Sky, g_Renderer, sunLightDirectionVec, [](Renderer* r, const std::string& value) {
        if (r) r->setSunDirection(Console::parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Sky, &g_World->getSky(), setRotAngle, GEN_FLOAT_SETTER(Sky, rot_angle));

    // Water
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), texLayer1, [](Water* w, const std::string& value) {
        w->setTexture(0, g_TextureMgr->load(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), texLayer2, [](Water* w, const std::string& value) {
        w->setTexture(1, g_TextureMgr->load(value));
    });
    
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), scrollDirection1, [](Water* w, const std::string& value) {
        w->setScrollDir(0, parseVec2(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), scrollDirection2, [](Water* w, const std::string& value) {
        w->setScrollDir(1, parseVec2(value));
    });

    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), scrollLayer1, [](Water* w, const std::string& value) {
        w->setScrollSpeed(0, parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), scrollLayer2, [](Water* w, const std::string& value) {
        w->setScrollSpeed(1, parseFloat(value));
    });

    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), tileLayer1, [](Water* w, const std::string& value) {
        w->setUVScale(0, parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), tileLayer2, [](Water* w, const std::string& value) {
        w->setUVScale(1, parseFloat(value));
    });

    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), color, [](Water* w, const std::string& value) {
        w->setColor(parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), deepColor, [](Water* w, const std::string& value) {
        w->setDeepColor(parseVec3(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), waterColorDepth, [](Water* w, const std::string& value) {
        w->setColorDepth(parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), waterAlphaDepth, [](Water* w, const std::string& value) {
        w->setAlphaDepth(parseFloat(value));
    });
    REGISTER_OBJECT_PROPERTY(Water, &g_World->getWater(), waterShallowAlpha, [](Water* w, const std::string& value) {
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
