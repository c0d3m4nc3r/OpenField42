#pragma once

#include "geometry/geometry.h"

class PatchTerrain : public Geometry
{
public:

    bool load(const GeometryTemplate* tmpl);

    int getSize() const;
    int getWorldSize() const;

    float getHeightAt(int x, int z) const;
    float getHeightAtWorld(float x, float z) const;

private:

    int size;
    int world_size;

    std::vector<float> heightmap;

};
