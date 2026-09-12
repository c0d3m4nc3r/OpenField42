#pragma once

struct GeometryTemplate;
class GeometryManager;
class Geometry;
class Shader;
class Sky
{
public:

    float rot_angle = 0.0f;

    bool init(const GeometryTemplate* tmpl);
    void clear();
    
    Geometry* getGeometry() const { return _geometry; }

private:

    std::string _geometry_name = "";
    Geometry* _geometry = nullptr;

};
