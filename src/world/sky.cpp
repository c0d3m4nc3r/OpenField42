#include "world/sky.h"

#include "geometry/geometry.h"
#include "geometry/template.h"
#include "render/shader.h"
#include "utils/log.h"

#include "glad/gl.h"

#include <glm/gtc/matrix_transform.hpp>

bool Sky::init()
{
    LOG_INFO("Sky::init: Initializing sky...");

    const GeometryTemplate* geom_tmpl = GeometryTemplate::current;

    if (!geom_tmpl)
    {
        LOG_ERROR("Sky::init: There is no active geometry template to init sky!");
        return false;
    }

    if (geom_tmpl->type != GeometryType::StandardMesh)
    {
        LOG_ERROR("Sky::init: Current geometry template type should be StandardMesh!");
        return false;
    }

    _geometry = Geometry::create(geom_tmpl);
    if (!_geometry)
    {
        LOG_ERROR("Sky::init: Failed to create sky geometry!");
        return false;
    }

    LOG_INFO("Sky::init: Sky initialized!");
    
    return true;
}

void Sky::shutdown()
{
    rot_angle = 0.0f;
    _geometry = nullptr;

    LOG_INFO("Sky::shutdown: Sky shutdown!");
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
