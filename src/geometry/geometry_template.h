#pragma once

#include "geometry/geometry.h"

#include <string>
#include <unordered_map>

struct GeometryTemplate
{
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
    
    static inline std::unordered_map<std::string, GeometryTemplate> registry;
    static inline GeometryTemplate* current = nullptr;

    static GeometryTemplate& create(const std::string& name, GeometryType type);
    static GeometryTemplate* get(const std::string& name);
};
