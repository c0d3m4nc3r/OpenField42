#pragma once

#include <glm/glm.hpp>

struct AABB;
struct Frustum
{
    glm::vec4 planes[6]; // left, right, bottom, top, near, far

    void extract(const glm::mat4& vp);
    bool intersects(const AABB& box) const;
};
