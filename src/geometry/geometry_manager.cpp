#include "geometry_manager.h"

#include "core/globals.h"
#include "geometry/geometry.h"
#include "geometry/geometry_template.h"
#include "geometry/standard_mesh.h"
#include "geometry/tree_mesh.h"

void GeometryManager::update(int uploads_per_frame)
{
    int uploaded = 0;

    while (uploaded < uploads_per_frame)
    {
        auto geo_opt = _geometries_to_upload.pop();
        if (!geo_opt.has_value()) break;

        geo_opt.value()->upload();
    }
}

Geometry* GeometryManager::createGeometry(const GeometryTemplate* tmpl)
{
    if (!tmpl)
    {
        LOG_ERROR("GeometryManager::createGeometry: Template is NULL!");
        return nullptr;
    }

    {
        auto it = _geometries.find(tmpl->name);
        if (it != _geometries.end())
            return it->second.get();
    }

    auto [it, _] = _geometries.try_emplace(tmpl->name);
    auto& geom = it->second;

    std::string type_str = geometryTypeToString(tmpl->type);

    switch (tmpl->type)
    {
    case GeometryType::StandardMesh:
        geom = std::make_unique<StandardMesh>(); break;
    case GeometryType::TreeMesh:
        geom = std::make_unique<TreeMesh>(); break;
    default:
        LOG_ERROR("GeometryManager::createGeometry: Unsupported geometry type: %s!",
            type_str.c_str());
        _geometries.erase(it);
        return nullptr;
    }

    g_ThreadPool.enqueue([this, &geom, tmpl] {
        if (geom->load(tmpl)) {
            _geometries_to_upload.push(geom.get());
        }
    });

    geom->type = tmpl->type;

    for (size_t i = 0; i < geom->lods.size(); ++i)
        geom->lods[i].distance = tmpl->lod_distances[i];

    return geom.get();
}

Geometry* GeometryManager::getGeometry(const std::string& name)
{
    auto it = _geometries.find(name);
    if (it == _geometries.end())
        return nullptr;
    return it->second.get();
}
