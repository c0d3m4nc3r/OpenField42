#pragma once

#include "geometry/geometry.h"

class Terrain;
class PatchTerrain : public Geometry
{
public:

    bool load(const GeometryTemplate* tmpl, Terrain& terrain);

};
