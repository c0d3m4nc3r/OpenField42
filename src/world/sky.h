#pragma once

class GeometryManager;
struct GeometryTemplate;
class Geometry;
class Shader;
class Sky
{
public:

    Sky(GeometryManager& geometry_mgr)
        : _geometry_mgr(geometry_mgr) {};

    float rot_angle = 0.0f;

    bool init(const GeometryTemplate* tmpl);
    void draw(Shader* shader) const;

private:

    GeometryManager& _geometry_mgr;
    Geometry* _geometry = nullptr;

};
