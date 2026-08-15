#include "world/world.h"

#include "core/globals.h"
#include "geometry/geometry_manager.h"
#include "geometry/geometry_template.h"
#include "object/object.h"
#include "object/object_template.h"
#include "render/renderer.h"
#include "world/sky.h"
#include "world/terrain.h"
#include "world/water.h"

void World::update(float dt)
{
    for (auto& obj : _objects)
        obj->update(dt);
}

Object* World::createObject(const ObjectTemplate* tmpl)
{
    if (!tmpl)
    {
        LOG_ERROR("World::createObject: template is NULL!");
        return nullptr;
    }
    
    auto obj = std::make_unique<Object>();
    obj->type = tmpl->type;
    obj->continous_rot_speed = tmpl->continous_rot_speed;

    Object* raw = obj.get();

    for (const auto& child : tmpl->children)
    {
        auto child_tmpl = ObjectTemplate::get(child.tmpl_name);
        if (!child_tmpl)
        {
            LOG_ERROR("World::createObject: Failed to create child: Object template with name '%s' not found!", child.tmpl_name.c_str());
            continue;
        }

        auto child_ptr = createObject(child_tmpl);
        if (!child_ptr)
        {
            LOG_ERROR("World::createObject: Failed to create child object!");
            continue;
        }

        child_ptr->setPosition(child.position);
        child_ptr->setRotation(child.rotation);
        child_ptr->parent = raw;
        obj->addChild(child_ptr);
    }

    if (tmpl->geometry.empty())
    {
        _objects.push_back(std::move(obj));
        return raw;
    }

    const GeometryTemplate* geom_tmpl = g_GeometryMgr->getTemplate(tmpl->geometry);
    if (!geom_tmpl)
    {
        LOG_ERROR("World::createObject: Geometry template with name '%s' not found!", tmpl->geometry.c_str());
        return nullptr;
    }

    // Added this to prevent spam in logs
    // Should be removed when the TreeMesh type will be supported
    // if (geom_tmpl->type == GeometryType::TreeMesh)
    // {
    //     _objects.push_back(std::move(obj));
    //     return raw;
    // }
    
    if (geom_tmpl->type == GeometryType::PatchTerrain)
    {
        _terrain.init(geom_tmpl);
        return nullptr;
    }

    auto* geometry = g_GeometryMgr->createGeometry(geom_tmpl);
    if (!geometry)
    {
        LOG_ERROR("World::createObject: Failed to load geometry!");
        return nullptr;
    }

    obj->setGeometry(geometry);

    _objects.push_back(std::move(obj));

    return raw;
}

void World::render()
{
    for (auto& obj : _objects)
    {
        auto* geom = obj->getGeometry();
        if (!geom) continue;

        g_Renderer->submit(geom, obj->getModelMatrix());
    }

    auto* terrain_geom = _terrain.getGeometry();
    if (terrain_geom) g_Renderer->submit(terrain_geom, glm::mat4(1.0f));

    auto* sky_geom = _sky.getGeometry();
    if (sky_geom)
    {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::rotate(transform, glm::radians(_sky.rot_angle), glm::vec3(0.0f, 1.0f, 0.0f));
        g_Renderer->submit(sky_geom, transform);
    }

    auto* water_geom = _water.getGeometry();
    if (water_geom) g_Renderer->submit(water_geom, glm::mat4(1.0f));
}
