#pragma once

#include "geometry/geometry.h"

class Camera;
class Renderer
{
public:

    struct Stats
    {
        size_t meshes_rendered = 0;
        size_t meshes_culled = 0;
        size_t polygons_rendered = 0;
        size_t polygons_culled = 0;
    };

    struct Lighting
    {
        Color diffuse;
        Color specular;
        Color ambient;
        Color global_ambient;
    } lighting;

    struct Fog
    {
        Color color = Color(0.5f, 0.5f, 0.5f, 1.0f);
        float start = 50.0f;
        float end = 200.0f;
        bool enabled = true;
    } fog;

    bool wireframe_mode = false;

    void submit(Geometry* geo, const glm::mat4& model);
    void flush();
    
    void resetStats();

    const Stats& getStats() const;

    void setCamera(Camera* camera);
    void setShader(std::shared_ptr<Shader> shader);

private:

    struct RenderItem
    {
        Geometry::Mesh* mesh;
        Geometry* geom;
        const glm::mat4* model;
        float distance_to_camera = 0.0f;
    };

    Camera* _camera = nullptr;
    
    std::shared_ptr<Shader> _shader;
    
    std::vector<RenderItem> _opaque_queue;
    std::vector<RenderItem> _transparent_queue;
    
    Stats _stats;

};

extern Renderer g_Renderer;