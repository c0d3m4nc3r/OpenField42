#include "math/aabb.h"

AABB::AABB() : min(0.0f), max(0.0f) {}
AABB::AABB(const glm::vec3& min, const glm::vec3& max)
    : min(min), max(max) {}

AABB AABB::transform(const glm::mat4& m) const
{
    glm::vec3 corners[8] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {min.x, max.y, min.z},
        {max.x, max.y, min.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {min.x, max.y, max.z},
        {max.x, max.y, max.z}
    };

    glm::vec3 new_min(FLT_MAX);
    glm::vec3 new_max(-FLT_MAX);

    for (int i = 0; i < 8; i++)
    {
        glm::vec4 world = m * glm::vec4(corners[i], 1.0f);
        new_min = glm::min(new_min, glm::vec3(world));
        new_max = glm::max(new_max, glm::vec3(world));
    }

    return AABB(new_min, new_max);
}

glm::vec3 AABB::center() const
{
    return (min + max) * 0.5f;
}