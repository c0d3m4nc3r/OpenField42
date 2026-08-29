#pragma once

#include "geometry/geometry.h"

class TreeMesh : public Geometry
{
public:
    bool load(const GeometryTemplate* tmpl) override;
};