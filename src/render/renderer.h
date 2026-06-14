#pragma once

#include "geometry/geometry.h"
#include "render/shader.h"
#include "world/world.h"

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

    bool init();
    void shutdown();
    void submit(Geometry* geom, const glm::mat4& model);
    void flush(World& world);
    
    void reloadShaders(Water& water);
    void resetStats();

    Camera* getCamera() const { return _camera; }
    
    const Stats& getStats() const { return _stats; }

    bool isWireframeEnabled() const { return _wireframe_enabled; }
    
    void setCamera(Camera* camera) { _camera = camera; }

    void setFogColor(const Color& color) { _fog.color = color; _fog_dirty = true; }
    void setFogStart(float start) { _fog.params.x = start; _fog_dirty = true; }
    void setFogEnd(float end) { _fog.params.y = end; _fog_dirty = true; }
    void setFogEnabled(bool enabled) { _fog.params.z = enabled ? 1.0f : 0.0f; _fog_dirty = true; }

    void setDiffuseLight(const Color& color) { _lighting.diffuse = color; _lighting_dirty = true; }
    void setSpecularLight(const Color& color) { _lighting.specular = color; _lighting_dirty = true; }
    void setAmbientLight(const Color& color) { _lighting.ambient = color; _lighting_dirty = true; }
    void setGlobalAmbientLight(const Color& color) { _lighting.global_ambient = color; _lighting_dirty = true; }
    void setSunDirection(const glm::vec3& dir) { _lighting.sun_dir = glm::vec4(dir, 1.0f); _lighting_dirty = true; }

    void setWireframeEnabled(bool enabled) { _wireframe_enabled = enabled; }

private:

    struct RenderItem
    {
        Geometry::Mesh* mesh;
        Geometry* geom;
        glm::mat4 model = glm::mat4(1.0f);
        float distance_to_camera = 0.0f;
    };

    struct alignas(16) UBO_CameraBlock
    {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 view_pos;
    };

    struct alignas(16) UBO_FogBlock
    {
        Color color = Color(0.5f, 0.5f, 0.5f, 1.0f);
        glm::vec4 params = {50.0f, 200.0f, 1.0f, 1.0f}; // x - start, y - end, z - enabled, w - padding
    };

    struct alignas(16) UBO_LightingBlock
    {
        Color diffuse;
        Color specular;
        Color ambient;
        Color global_ambient;
        glm::vec4 sun_dir; // xyz - direction, w - padding
    };

    struct {
        std::unique_ptr<Shader> standard;
        std::unique_ptr<Shader> sky;
        std::unique_ptr<Shader> water;
    } _shaders;
    
    std::vector<RenderItem> _opaque_queue;
    std::vector<RenderItem> _transparent_queue;

    bool _wireframe_enabled = false;
    
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
    void skyPass(const Sky& sky);
    void waterPass(const Water& water);
};
