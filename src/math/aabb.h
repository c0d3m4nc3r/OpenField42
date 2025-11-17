#pragma once

#include <glm/glm.hpp>

struct AABB
{
    glm::vec3 min, max;

    AABB();
    AABB(const glm::vec3& min, const glm::vec3& max);

    AABB transform(const glm::mat4& m) const;
    glm::vec3 center() const;
};
