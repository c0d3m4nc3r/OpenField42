#include "geometry_manager.h"

#include "geometry/geometry.h"
#include "geometry/geometry_template.h"
#include "geometry/standard_mesh.h"
#include "geometry/tree_mesh.h"


Geometry* GeometryManager::createGeometry(const GeometryTemplate* tmpl, bool upload)
{
    if (!tmpl)
    {
        LOG_ERROR("GeometryManager::createGeometry: Template is NULL!");
        return nullptr;
    }

    auto it = _geometries.find(tmpl->name);
    if (it != _geometries.end())
        return it->second.get();

    Geometry* geom = nullptr;

    switch (tmpl->type)
    {
        case GeometryType::StandardMesh:
        {
            StandardMesh* stdmesh_geom = new StandardMesh();
            geom = stdmesh_geom;
            if (!stdmesh_geom->load(tmpl))
            {
                LOG_ERROR("GeometryManager::createGeometry: Failed to load StandardMesh '%s'!", tmpl->name.c_str());
                delete stdmesh_geom;
                return nullptr;
            }
        } break;
        case GeometryType::TreeMesh:
        {
            TreeMesh* treemesh_geom = new TreeMesh();
            geom = treemesh_geom;
            if (!treemesh_geom->load(tmpl))
            {
                LOG_ERROR("GeometryManager::createGeometry: Failed to load TreeMesh '%s'!", tmpl->name.c_str());
                delete treemesh_geom;
                return nullptr;
            } break;
        }
        default:
        {
            LOG_ERROR("GeometryManager::createGeometry: Unsupported geometry type: %s!",
                geometryTypeToString(tmpl->type).c_str());
            return nullptr;
        }
    }

    geom->type = tmpl->type;

    for (size_t i = 0; i < geom->lods.size(); ++i)
    {
        geom->lods[i].distance = tmpl->lod_distances[i];
    }

    if (upload && !geom->upload())
    {
        LOG_ERROR("GeometryManager::createGeometry: Failed to upload %s '%s'!", geometryTypeToString(tmpl->type).c_str(), tmpl->name.c_str());
        delete geom;
        return nullptr;
    }

    _geometries[tmpl->name] = std::unique_ptr<Geometry>(geom);

    return geom;
}

Geometry* GeometryManager::getGeometry(const std::string& name)
{
    auto it = _geometries.find(name);
    if (it == _geometries.end())
        return nullptr;
    return it->second.get();
}
