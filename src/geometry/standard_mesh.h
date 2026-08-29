#pragma once

#include "geometry/geometry.h"

class StandardMesh : public Geometry
{
public:

    bool load(const GeometryTemplate* tmpl) override;

private:

    bool loadMaterials(const GeometryTemplate* tmpl);

};
