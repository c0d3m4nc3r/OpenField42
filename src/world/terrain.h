#pragma once

#include <algorithm>

struct GeometryTemplate;

class Geometry;

class Terrain
{
public:

    bool init(const GeometryTemplate* tmpl);
    void shutdown();

    int getSize() const { return _size; }
    int getWorldSize() const { return _world_size; }
    Geometry* getGeometry() const { return _geometry.get(); }
    int getWaterHeight() const { return _water_height; }

    float getHeight(int x, int z) const
    {
        x = std::clamp(x, 0, _size - 1);
        z = std::clamp(z, 0, _size - 1);
        return _heights[z * _size + x];
    }

    void setSize(int size)
    {
        _size = size;
        _heights.resize(size * size, 0.0f);
    }

    void setWorldSize(int size) { _world_size = size; }
    void setWaterHeight(int height) { _water_height = height; }
    
    void setHeight(int x, int z, float value)
    {
        if (x < 0 || x >= _size || z < 0 || z >= _size)
            return;
        _heights[z * _size + x] = value;
    }

private:

    int _size = 0, _world_size = 0;
    int _water_height = 0;
    std::unique_ptr<Geometry> _geometry;
    std::vector<float> _heights = {};
};
