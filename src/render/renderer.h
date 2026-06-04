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

    bool wireframe_mode = false;

    bool init();
    void shutdown();
    void submit(Geometry* geo, const glm::mat4& model);
    void flush();
    
    void resetStats();

    const Stats& getStats() const { return _stats; }
    
    void setCamera(Camera* camera) { _camera = camera; }

    void setFogColor(const Color& color) { _fog.color = color; _fog_dirty = true; }
    void setFogStart(float start) { _fog.start = start; _fog_dirty = true; }
    void setFogEnd(float end) { _fog.end = end; _fog_dirty = true; }
    void setFogEnabled(bool enabled) { _fog.enabled = enabled; _fog_dirty = true; }

    void setDiffuseLight(const Color& color) { _lighting.diffuse = color; _lighting_dirty = true; }
    void setSpecularLight(const Color& color) { _lighting.specular = color; _lighting_dirty = true; }
    void setAmbientLight(const Color& color) { _lighting.ambient = color; _lighting_dirty = true; }
    void setGlobalAmbientLight(const Color& color) { _lighting.global_ambient = color; _lighting_dirty = true; }

private:

    struct RenderItem
    {
        Geometry::Mesh* mesh;
        Geometry* geom;
        const glm::mat4* model;
        float distance_to_camera = 0.0f;
    };

    struct UBO_CameraBlock
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec3 view_pos;
        float padding; // for std140 alignment
    };

    struct UBO_FogBlock
    {
        Color color = Color(0.5f, 0.5f, 0.5f, 1.0f);
        float start = 50.0f;
        float end = 200.0f;
        bool enabled = true;
        float padding; // for std140 alignment
    };

    struct alignas(16) UBO_LightingBlock
    {
        Color diffuse;
        Color specular;
        Color ambient;
        Color global_ambient;
    };

    struct {
        std::shared_ptr<Shader> standard;
        std::shared_ptr<Shader> sky;
        std::shared_ptr<Shader> water;
    } _shaders;
    
    std::vector<RenderItem> _opaque_queue;
    std::vector<RenderItem> _transparent_queue;
    
    Camera* _camera = nullptr;
    unsigned int _camera_ubo = 0;
    
    UBO_FogBlock _fog;
    unsigned int _fog_ubo = 0;
    bool _fog_dirty = true;

    UBO_LightingBlock _lighting;
    unsigned int _lighting_ubo = 0;
    bool _lighting_dirty = true;
    
    Stats _stats;

    void opaquePass();
    void transparentPass();
    void skyPass();
    void waterPass();
};

extern Renderer g_Renderer;