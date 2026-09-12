#include "object/object_manager.h"

#include "core/console.h"
#include "core/globals.h"
#include "core/template_manager.h"
#include "geometry/geometry_manager.h"
#include "geometry/geometry_template.h"
#include "object/object.h"
#include "object/object_template.h"
#include "render/renderer.h"
#include "world/world.h"

void ObjectManager::registerCmds() const
{
    g_Console->registerCmd("Object.create", [](Console::ExecContext& ctx, const Console::CommandArgs& args) -> CommandResult
    {
        ctx.last_obj = nullptr;

        std::string tmpl_name = std::string(args[0]);

        if (args.empty())
            return CommandResult{ "Not enough arguments! Usage: Object.create <template_name>", CommandStatus::Error };

        auto* tmpl = g_TemplateMgr->get<ObjectTemplate>(args[0]);
        if (!tmpl)
            return CommandResult{ "Object template with name '" + tmpl_name + "' not found!", CommandStatus::Warning };

        ctx.last_obj = g_ObjectMgr->createObject(tmpl);
        if (!ctx.last_obj)
        {
            // PatchTerrain initializes level-wide terrain globally and never creates an Object (always returns nullptr).
            auto* geom_tmpl = g_TemplateMgr->get<GeometryTemplate>(tmpl->geometry);
            if (geom_tmpl && geom_tmpl->type == GeometryType::PatchTerrain)
                return {};

            return CommandResult{ "Failed to create object from template '" + tmpl_name + "'!", CommandStatus::Error };
        }

        return {};
    });

    g_Console->bindContextProperty("Object.absolutePosition", &Console::Console::ExecContext::last_obj, &Object::getPosition, &Object::setPosition);
    g_Console->bindContextProperty("Object.rotation", &Console::Console::ExecContext::last_obj, &Object::getRotation, &Object::setRotation);
    g_Console->bindContextProperty("Object.scale", &Console::Console::ExecContext::last_obj, &Object::getScale, &Object::setScale);

    g_Console->registerCmd("ObjectTemplate.create", [](Console::ExecContext& ctx, const Console::CommandArgs& args) -> CommandResult
    {
        if (args.size() < 2)
        {
            LOG_ERROR("Console: ObjectTemplate.create: Not enough arguments!");
            return CommandResult{ "Not enough arguments! Usage: ObjectTemplate.create <type> <name>", CommandStatus::Error };
        }

        ObjectType type = objectTypeFromString(args[0]);

        std::string tmpl_name = std::string(args[0]);

        if (type == ObjectType::Unknown)
        {
            LOG_WARNING("Console: ObjectTemplate.create: Unknown object type '%s'!", tmpl_name.c_str());
            return CommandResult{ "Unknown object type '" + tmpl_name + "'", CommandStatus::Warning };
        }

        ctx.last_obj_tmpl = g_TemplateMgr->create<ObjectTemplate>(args[1], type);

        return {};
    });

    g_Console->registerCmd("ObjectTemplate.addTemplate", [](Console::ExecContext& ctx, const Console::CommandArgs& args) -> CommandResult
    {
        if (args.empty())
        {
            return CommandResult{ "Not enough arguments! Usage: ObjectTemplate.addTemplate <name>", CommandStatus::Error };
        }

        auto current = ctx.last_obj_tmpl;
        if (current)
        {
            ctx.last_child = &current->children.emplace_back(
                std::string(args[0]), glm::vec3(0.0f), glm::vec3(0.0f)
            );
        }

        return {};
    });

    g_Console->bindContextProperty("ObjectTemplate.geometry", &Console::Console::ExecContext::last_obj_tmpl, &ObjectTemplate::geometry);
    g_Console->bindContextProperty("ObjectTemplate.setPosition", &Console::ExecContext::last_child, &ObjectTemplate::Child::position);
    g_Console->bindContextProperty("ObjectTemplate.setRotation", &Console::ExecContext::last_child, &ObjectTemplate::Child::rotation);
    g_Console->bindContextProperty("ObjectTemplate.setContinousRotationSpeed", &Console::Console::ExecContext::last_obj_tmpl, &ObjectTemplate::continous_rot_speed);
}

Object* ObjectManager::createObject(const ObjectTemplate* tmpl)
{
    if (!tmpl)
    {
        LOG_ERROR("ObjectManager::createObject: template is NULL!");
        return nullptr;
    }

    auto obj = std::make_unique<Object>();
    obj->type = tmpl->type;
    obj->continous_rot_speed = tmpl->continous_rot_speed;

    for (const auto& child : tmpl->children)
    {
        auto child_tmpl = g_TemplateMgr->get<ObjectTemplate>(child.tmpl_name);
        if (!child_tmpl)
        {
            LOG_ERROR("ObjectManager::createObject: Failed to create child: Object template '%s' not found!", child.tmpl_name.c_str());
            continue;
        }

        auto child_ptr = createObject(child_tmpl);
        if (!child_ptr)
        {
            LOG_ERROR("ObjectManager::createObject: Failed to create child object!");
            continue;
        }

        child_ptr->setPosition(child.position);
        child_ptr->setRotation(child.rotation);
        
        child_ptr->parent = obj.get();
        obj->addChild(child_ptr);
    }

    if (!tmpl->geometry.empty())
    {
        auto* geom_tmpl = g_TemplateMgr->get<GeometryTemplate>(tmpl->geometry);
        if (!geom_tmpl)
        {
            LOG_ERROR("ObjectManager::createObject: Geometry template '%s' not found!", tmpl->geometry.c_str());
            return nullptr;
        }

        if (geom_tmpl->type == GeometryType::PatchTerrain)
        {
            g_World->getTerrain().init(geom_tmpl);
            return nullptr;
        }

        auto* geometry = g_GeometryMgr->createGeometry(geom_tmpl);
        if (!geometry)
        {
            LOG_ERROR("ObjectManager::createObject: Failed to load geometry!");
            return nullptr;
        }

        obj->setGeometry(geometry);
    }

    Object* raw_ptr = obj.get();
    _objects.push_back(std::move(obj));
    return raw_ptr;
}

void ObjectManager::clearObjects()
{
    _objects.clear();
}

void ObjectManager::updateObjects(float dt)
{
    for (auto& obj : _objects)
        obj->update(dt);
}

void ObjectManager::renderObjects()
{
    for (auto& obj : _objects)
    {
        auto* geom = obj->getGeometry();
        if (!geom) continue;

        g_Renderer->submit(geom, obj->getModelMatrix());
    }
}
