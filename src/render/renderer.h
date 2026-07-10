#pragma once

#include "geometry/geometry.h"
#include "render/render_context.h"
#include "render/render_pass.h"
#include "render/shader.h"

class Camera;
class Renderer
{
public:

    struct WaterParams
    {
        glm::vec4 layer_1; // xy dir, z speed, w uv_scale
        glm::vec4 layer_2;
        Texture* tex_layer1 = nullptr;
        Texture* tex_layer2 = nullptr;
    };

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
    void flush();
    
    void reloadShaders();
    void resetStats();

    Camera* getCamera() const { return _camera; }
    
    const Stats& getStats() const { return _stats; }

    bool isWireframeEnabled() const { return _context.wireframe_enabled; }
    
    void setViewport(int x, int y, int w, int h) const;
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

    void setWaterParams(const WaterParams& params)
    {
        _water.layer_1 = params.layer_1;
        _water.layer_2 = params.layer_2;
        _water_textures[0] = params.tex_layer1;
        _water_textures[1] = params.tex_layer2;
        _water_dirty = true;
    }

    void setWireframeEnabled(bool enabled) { _context.wireframe_enabled = enabled; }

private:

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

    struct alignas(16) UBO_WaterBlock
    {
        // xy = scroll dir, z = scroll speed, w = uv scale
        glm::vec4 layer_1 = glm::vec4(glm::vec3(0.0f), 1.0f);
        glm::vec4 layer_2 = glm::vec4(glm::vec3(0.0f), 1.0f);
    };

    struct {
        std::unique_ptr<Shader> standard;
        std::unique_ptr<Shader> sky;
        std::unique_ptr<Shader> water;
    } _shaders;
    
    std::array<std::unique_ptr<RenderPass>, static_cast<size_t>(RenderPass::Type::Count)> _passes;
    const RenderPass::Type _execution_order[static_cast<size_t>(RenderPass::Type::Count)] = {
        RenderPass::Type::Opaque,
        RenderPass::Type::Sky,
        RenderPass::Type::Water,
        RenderPass::Type::Transparent
    };

    RenderContext _context;
    
    Camera* _camera = nullptr;
    unsigned int _camera_ubo = 0;
    
    UBO_FogBlock _fog;
    unsigned int _fog_ubo = 0;
    bool _fog_dirty = true;

    UBO_LightingBlock _lighting;
    unsigned int _lighting_ubo = 0;
    bool _lighting_dirty = true;

    UBO_WaterBlock _water;
    unsigned int _water_ubo = 0;
    bool _water_dirty = true;

    Texture* _water_textures[2] = { nullptr, nullptr };
    
    Stats _stats;

    RenderPass* getPass(RenderPass::Type type);
};
