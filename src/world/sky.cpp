#include "world/sky.h"

#include "geometry/geometry_manager.h"
#include "geometry/geometry_template.h"
#include "geometry/geometry.h"
#include "render/shader.h"
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

    if (tmpl->type != GeometryType::StandardMesh)
    {
        LOG_ERROR("Sky::init: Geometry template is not StandardMesh!");
        return false;
    }

    _geometry = _geometry_mgr.createGeometry(tmpl);
    if (!_geometry)
    {
        LOG_ERROR("Sky::init: Failed to create sky geometry!");
        return false;
    }

    LOG_INFO("Sky::init: Sky initialized!");
    
    return true;
}

void Sky::draw(Shader* shader) const
{
    if (!shader || !_geometry) return;

    shader->use();
    
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    _geometry->draw(shader, glm::rotate(glm::mat4(1.0f), glm::radians(rot_angle), glm::vec3(0.0f, 1.0f, 0.0f)));
    
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);    
}
