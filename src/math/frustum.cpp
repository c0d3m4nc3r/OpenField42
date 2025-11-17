#include "math/frustum.h"
#include "math/aabb.h"

void Frustum::extract(const glm::mat4& vp)
{
    // Left
    planes[0] = glm::vec4(vp[0][3] + vp[0][0],
                            vp[1][3] + vp[1][0],
                            vp[2][3] + vp[2][0],
                            vp[3][3] + vp[3][0]);

    // Right
    planes[1] = glm::vec4(vp[0][3] - vp[0][0],
                            vp[1][3] - vp[1][0],
                            vp[2][3] - vp[2][0],
                            vp[3][3] - vp[3][0]);

    // Bottom
    planes[2] = glm::vec4(vp[0][3] + vp[0][1],
                            vp[1][3] + vp[1][1],
                            vp[2][3] + vp[2][1],
                            vp[3][3] + vp[3][1]);

    // Top
    planes[3] = glm::vec4(vp[0][3] - vp[0][1],
                            vp[1][3] - vp[1][1],
                            vp[2][3] - vp[2][1],
                            vp[3][3] - vp[3][1]);

    // Near
    planes[4] = glm::vec4(vp[0][3] + vp[0][2],
                            vp[1][3] + vp[1][2],
                            vp[2][3] + vp[2][2],
                            vp[3][3] + vp[3][2]);

    // Far
    planes[5] = glm::vec4(vp[0][3] - vp[0][2],
                            vp[1][3] - vp[1][2],
                            vp[2][3] - vp[2][2],
                            vp[3][3] - vp[3][2]);

    for (int i = 0; i < 6; i++)
    {
        float length = glm::length(glm::vec3(planes[i]));
        planes[i] /= length;
    }
}

bool Frustum::intersects(const AABB& box) const
{
    for (int i = 0; i < 6; i++)
    {
        const glm::vec3 normal = glm::vec3(planes[i]);
        float d = planes[i].w;

        glm::vec3 p = box.min;
        if (normal.x >= 0) p.x = box.max.x;
        if (normal.y >= 0) p.y = box.max.y;
        if (normal.z >= 0) p.z = box.max.z;

        if (glm::dot(normal, p) + d < 0.0f)
            return false;
    }
    return true;
}