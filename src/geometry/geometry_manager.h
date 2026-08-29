#pragma once

#include "core/thread_safe_queue.h"
#include "geometry/geometry_template.h"

class Geometry;
enum class GeometryType : unsigned char;

class GeometryManager
{
public:

    void update(int uploads_per_frame);

    Geometry* createGeometry(const GeometryTemplate* tmpl);
    Geometry* getGeometry(const std::string& name);

private:

    std::unordered_map<std::string, std::unique_ptr<Geometry>> _geometries;
    ThreadSafeQueue<Geometry*> _geometries_to_upload;
};
