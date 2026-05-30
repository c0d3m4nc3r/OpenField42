#pragma once

#include <glm/vec3.hpp>

class Geometry;
class Shader;
class Sky
{
public:

    glm::vec3 sun_light_dir = glm::vec3(0.0f, -1.0f, 0.0f);
    float rot_angle = 0.0f;

    bool init();
    void draw(Shader* shader);

private:

    Geometry* geometry = nullptr;

};

extern Sky sky;
