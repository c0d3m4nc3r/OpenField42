#pragma once

#include "geometry/geometry.h"

#include <glm/vec2.hpp>

#include <memory>

class Texture;
class Water
{
public:

    bool init();
    void shutdown();
    void draw(Shader* shader);

    void setTexture(int layer, std::shared_ptr<Texture> texture)
    {
        _layers[layer].texture = std::move(texture);
        _dirty = true;
    }

    void setScrollDir(int layer, const glm::vec2& scroll_dir)
    {
        _layers[layer].scroll_dir = scroll_dir;
        _dirty = true;
    }

    void setScrollSpeed(int layer, float scroll_speed)
    {
        _layers[layer].scroll_speed = scroll_speed;
        _dirty = true;
    }

    void setColor(const Color& color) { _color = color; }
    void setDeepColor(const Color& color) { _deep_color = color; }

private:

    struct UBO_WaterBlock
    {
        glm::vec4 scroll_1;   // xy = dir, z = speed, w = padding
        glm::vec4 scroll_2;   // xy = dir, z = speed, w = padding
    };

    struct Layer
    {
        std::shared_ptr<Texture> texture;
        glm::vec2 scroll_dir;
        float scroll_speed;
    };

    Layer _layers[2];

    Color _color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color _deep_color = Color(0.3f, 0.3f, 0.3f, 1.0f);

    unsigned int _water_ubo = 0;

    bool _ubo_bound = false;
    bool _dirty = true;

    Geometry::Mesh _mesh;

};

extern Water g_Water;
