#pragma once

#include "geometry/geometry.h"

#include <glm/vec2.hpp>

class Terrain;
class Texture;
class Water
{
public:

    struct Layer
    {
        TextureHandle texture;
        glm::vec2 scroll_dir{0.0f};
        float scroll_speed = 0.0f;
        float uv_scale = 1.0f;
    };

    void clear();

    Geometry* getGeometry()
    {
        if (_geometry_dirty)
        {
            generateGeometry();
            _geometry_dirty = false;
        }
        return &_geometry;
    }

    unsigned int getHalfVecLUT()
    {
        if (_halfvec_lut_dirty)
        {
            generateHalfVecLUT();
            _halfvec_lut_dirty = false;
        }
        return _halfvec_lut;
    }

    Layer& getLayer(int index) { return _layers[index]; }

    const glm::vec3& getColor() const { return _color; }
    const glm::vec3& getDeepColor() const { return _deep_color; }
    const glm::vec3& getSpecularColor() const { return _specular_color; }
    float getColorDepth() const { return _color_depth; }
    float getAlphaDepth() const { return _alpha_depth; }
    float getShallowAlpha() const { return _shallow_alpha; }
    float getSpecularStreakFactor() const { return _specular_streak_factor; }

    bool isDirty() const { return _dirty; }
    void clearDirty() { _dirty = false; }

    void setHalfVecLUTDirty(bool dirty) { _halfvec_lut_dirty = dirty; }

    void setTexture(int layer, const TextureHandle& texture)
    {
        _layers[layer].texture = texture;
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

    void setColor(const glm::vec3& color) { _color = color; _geometry_dirty = true; }
    void setDeepColor(const glm::vec3& color) { _deep_color = color; _geometry_dirty = true; }
    void setSpecularColor(const glm::vec3& color) { _specular_color = color; _dirty = true; }
    void setColorDepth(float depth) { _color_depth = depth; _geometry_dirty = true; }
    void setAlphaDepth(float depth) { _alpha_depth = depth; _geometry_dirty = true; }
    void setShallowAlpha(float alpha) { _shallow_alpha = alpha; _geometry_dirty = true; }
    void setSpecularStreakFactor(float factor) { _specular_streak_factor = factor; _halfvec_lut_dirty = true; }

private:

    Layer _layers[3]; // third is normal map

    Geometry _geometry;
    
    // Half-vector lookup for water specular
    unsigned int _halfvec_lut = 0;
    
    glm::vec3 _color = glm::vec3(1.0f);
    glm::vec3 _deep_color = glm::vec3(0.3f);
    glm::vec3 _specular_color = glm::vec3(0.5f);
    float _color_depth = 5.0f;
    float _alpha_depth = 1.0f;
    float _shallow_alpha = 0.5f;
    float _specular_streak_factor = 0.001f;

    bool _dirty = true;
    bool _geometry_dirty = true;
    bool _halfvec_lut_dirty = true; // set by renderer!
    
    void generateGeometry();
    void generateHalfVecLUT();
};
