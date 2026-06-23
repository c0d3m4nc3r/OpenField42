#pragma once

#include "geometry/geometry_template.h"

class Geometry;
enum class GeometryType : unsigned char;

class GeometryManager
{
public:

    Geometry* createGeometry(const GeometryTemplate* tmpl, bool upload = true);
    GeometryTemplate* createTemplate(const std::string& name, GeometryType type);
    
    Geometry* getGeometry(const std::string& name);
    GeometryTemplate* getTemplate(const std::string& name);

private:

    std::unordered_map<std::string, std::unique_ptr<Geometry>> _geometries;
    std::unordered_map<std::string, GeometryTemplate> _templates;
};
