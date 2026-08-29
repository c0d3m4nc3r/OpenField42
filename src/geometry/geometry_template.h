#pragma once

#include "geometry/geometry_type.h"

#include <array>
#include <string>

struct GeometryTemplate
{
    GeometryTemplate(const std::string& name, GeometryType type = GeometryType::Unknown)
        : name(name), type(type) {}

    std::string name;
    GeometryType type = GeometryType::Unknown;

    std::string file;
    std::string material_map;
    std::string tex_base_name;
    std::string detail_tex_name;
    int material_size;
    int world_size;
    int tex_offset_x;
    int tex_offset_y;
    int water_level;
    float y_scale;

    std::array<float, 6> lod_distances;
};
