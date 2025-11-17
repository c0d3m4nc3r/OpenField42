#pragma once

#include "geometry/material.h"

#include <glm/glm.hpp>

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

    std::shared_ptr<Texture> depth_map;
    float min_depth = 0.0f;
    float max_depth = 0.0f;

};

extern Water water;
