#pragma once

#include <glm/vec3.hpp>

class Geometry;
class Shader;
class Sky
{
public:

    float rot_angle = 0.0f;

    bool init();
    void shutdown();
    void draw(Shader* shader) const;

private:

    Geometry* _geometry = nullptr;

};
