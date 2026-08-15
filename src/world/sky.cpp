#include "world/sky.h"

#include "core/globals.h"
#include "geometry/geometry_manager.h"
#include "geometry/geometry.h"
#include "utils/log.h"

#include "glad/gl.h"

#include <glm/gtc/matrix_transform.hpp>

bool Sky::init(const GeometryTemplate* tmpl)
{
    LOG_INFO("Sky::init: Initializing sky...");

    if (!tmpl)
    {
        LOG_ERROR("Sky::init: Geometry template is NULL!");
        return false;
    }

    _geometry = g_GeometryMgr->createGeometry(tmpl);
    if (!_geometry)
    {
        LOG_ERROR("Sky::init: Failed to create sky geometry!");
        return false;
    }

    _geometry->type = GeometryType::SkyMesh;

    LOG_INFO("Sky::init: Sky initialized!");
    
    return true;
}

