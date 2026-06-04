#pragma once

#include "geometry/geometry.h"

#include <glm/vec2.hpp>

#include <memory>

class Texture;
class Water
{
public:

    struct Layer
    {
        std::shared_ptr<Texture> texture;
        glm::vec2 scroll_dir;
        float scroll_speed;
    };

    // struct NormalMap {
    //     std::shared_ptr<Texture> texture;
    //     glm::vec2 scroll_dir;
    //     float scroll_speed;
    // } normal_map;

    Layer layers[2];

    Color shallow_color;
    Color deep_color;
    float alpha_depth = 1.0f;

    bool init(float size, float world_size, float height);
    void shutdown();
    void draw(Shader* shader);

private:

    Geometry::Mesh _mesh;

};

extern Water g_Water;
