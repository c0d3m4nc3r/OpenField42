#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct AABB
{
    glm::vec3 min, max;

    AABB();
    AABB(const glm::vec3& min, const glm::vec3& max);

    AABB transform(const glm::mat4& m) const;
    glm::vec3 center() const;
};
