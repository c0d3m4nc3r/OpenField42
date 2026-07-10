#pragma once

#include "geometry/geometry.h"

#include <glm/vec2.hpp>

#include <memory>

class Terrain;
class Texture;
class Water
{
public:

    struct Layer
    {
        std::shared_ptr<Texture> texture = nullptr;
        glm::vec2 scroll_dir{0.0f};
        float scroll_speed = 0.0f;
        float uv_scale = 1.0f;
    };

    bool init(const Terrain& terrain);
    void shutdown();

    Geometry* getGeometry() { return &_geometry; }

    Layer& getLayer(int index) { return _layers[index]; }

    bool isDirty() const { return _dirty; }
    void clearDirty() { _dirty = false; }

    void setTexture(int layer, std::shared_ptr<Texture> texture)
    {
        _layers[layer].texture = std::move(texture);
        _dirty = true;
    }

    void setScrollDir(int layer, const glm::vec2& dir)
    {
        _layers[layer].scroll_dir = dir;
        _dirty = true;
    }

    void setScrollSpeed(int layer, float speed)
    {
        _layers[layer].scroll_speed = speed;
        _dirty = true;
    }

    void setUVScale(int layer, float scale)
    {
        _layers[layer].uv_scale = scale;
        _dirty = true;
    }

    void setColor(const Color& color) { _color = color; }
    void setDeepColor(const Color& color) { _deep_color = color; }
    void setColorDepth(float depth) { _color_depth = depth; }
    void setAlphaDepth(float depth) { _alpha_depth = depth; }
    void setShallowAlpha(float alpha) { _shallow_alpha = alpha; }

private:

    Layer _layers[2];

    Color _color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color _deep_color = Color(0.3f, 0.3f, 0.3f, 1.0f);
    float _color_depth = 5.0f;
    float _alpha_depth = 1.0f;
    float _shallow_alpha = 0.5f;

    bool _dirty = true;

    Geometry _geometry;
};
