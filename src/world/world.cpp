#include "world/world.h"

#include "geometry/template.h"
#include "object/object.h"
#include "object/template.h"
#include "render/renderer.h"
#include "world/sky.h"
#include "world/terrain.h"
#include "world/water.h"

World::World() {}
World::~World() = default;

void World::init()
{
    _sky = std::make_unique<Sky>();
    _terrain = std::make_unique<Terrain>();
    _water = std::make_unique<Water>();

    LOG_INFO("World::init: World initialized!");
}

void World::shutdown()
{
    _sky->shutdown();
    _sky.reset();

    _terrain->shutdown();
    _terrain.reset();
    
    _water->shutdown();
    _water.reset();

    _objects.clear();

    LOG_INFO("World::shutdown: World shutdown!");
}

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

    const GeometryTemplate* geom_tmpl = GeometryTemplate::get(tmpl->geometry);
    if (!geom_tmpl)
    {
        LOG_ERROR("World::createObject: Geometry template with name '%s' not found!", tmpl->geometry.c_str());
        return nullptr;
    }

    // Added this to prevent spam in logs
    // Should be removed when the TreeMesh type will be supported
    if (geom_tmpl->type == GeometryType::TreeMesh)
    {
        _objects.push_back(std::move(obj));
        return raw;
    }
    
    if (geom_tmpl->type == GeometryType::PatchTerrain)
    {
        _terrain->init(geom_tmpl);
        return nullptr;
    }

    auto* geometry = Geometry::create(geom_tmpl);
    if (!geometry)
    {
        LOG_ERROR("World::createObject: Failed to load geometry!");
        return nullptr;
    }

    obj->setGeometry(geometry);

    _objects.push_back(std::move(obj));

    return raw;
}

void World::renderObjects(Renderer& renderer) const
{
    for (auto& obj : _objects)
    {
        auto* geom = obj->getGeometry();
        if (!geom) continue;

        renderer.submit(geom, obj->getModelMatrix());
    }
}

void World::renderTerrain(Renderer& renderer) const
{
    auto* geom = _terrain->getGeometry();
    if (!geom) return;
    
    renderer.submit(geom, glm::mat4(1.0f));
}
