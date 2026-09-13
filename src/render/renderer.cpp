#include "render/renderer.h"

#include "core/globals.h"
#include "core/config.h"
#include "core/console.h"
#include "render/camera.h"
#include "render/render_passes.h"
#include "render/shader.h"
#include "render/shader_manager.h"
#include "world/sky.h"

#include "glad/gl.h"

#include <SDL3/SDL_timer.h>

#include <format>

bool Renderer::init()
{
    LOG_INFO("Renderer::init: Initializing renderer...");

    Shader* sky_shader = g_ShaderMgr->get("sky");
    Shader* standard_shader = g_ShaderMgr->get("standard");
    Shader* terrain_shader = g_ShaderMgr->get("terrain");
    Shader* water_shader = g_ShaderMgr->get("water");

    // Create and bind UBOs

    glCreateBuffers(1, &_camera_ubo);
    glNamedBufferStorage(_camera_ubo, sizeof(UBO_CameraBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, _camera_ubo);
    
    glCreateBuffers(1, &_fog_ubo);
    glNamedBufferStorage(_fog_ubo, sizeof(UBO_FogBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, _fog_ubo);

    glCreateBuffers(1, &_lighting_ubo);
    glNamedBufferStorage(_lighting_ubo, sizeof(UBO_LightingBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, _lighting_ubo);
    
    glCreateBuffers(1, &_water_ubo);
    glNamedBufferStorage(_water_ubo, sizeof(UBO_WaterBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, _water_ubo);

    // Create passes

    createPass<StandardOpaquePass>(RenderPass::Type::Standard_Opaque, standard_shader);
    createPass<StandardTransparentPass>(RenderPass::Type::Standard_Transparent, standard_shader);
    createPass<TreeOpaquePass>(RenderPass::Type::Tree_Opaque, standard_shader);
    createPass<TreeTransparentPass>(RenderPass::Type::Tree_Transparent, standard_shader);
    createPass<TerrainPass>(RenderPass::Type::Terrain, terrain_shader);
    createPass<SkyPass>(RenderPass::Type::Sky, sky_shader);
    createPass<WaterPass>(RenderPass::Type::Water, water_shader);

    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK); glFrontFace(GL_CW);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    LOG_INFO("Renderer::init: Renderer initialized!");

    return true;
}

void Renderer::shutdown()
{
    LOG_INFO("Renderer::shutdown: Shutting down renderer...");

    for (auto& pass : _passes)
        pass.reset();

    if (_camera_ubo) glDeleteBuffers(1, &_camera_ubo);
    if (_fog_ubo) glDeleteBuffers(1, &_fog_ubo);
    if (_lighting_ubo) glDeleteBuffers(1, &_lighting_ubo);
    if (_water_ubo) glDeleteBuffers(1, &_water_ubo);

    LOG_INFO("Renderer::shutdown: Renderer shutdown!");
}

static inline float distanceToAABB(const glm::vec3& point, const AABB& box)
{
    glm::vec3 closest = glm::clamp(point, glm::min(box.min, box.max), glm::max(box.min, box.max));
    return glm::distance(point, closest);
}

static inline int selectLOD(float distance, const Geometry& geom)
{
    if (geom.lods.empty()) return 0;
    if (geom.lods.size() == 1) return 0;

    for (int i = static_cast<int>(geom.lods.size()) - 1; i >= 1; --i) {
        float lod_dist = geom.lods[i].distance * 5.0f;
        if (distance >= lod_dist && lod_dist > 0.0f) {
            return i;
        }
    }

    return 0;
}

static inline RenderPass::Type selectPassType(GeometryType geom_type, bool transparent)
{
    switch (geom_type)
    {
    case GeometryType::StandardMesh:
        if (transparent) {
            return RenderPass::Type::Standard_Transparent;
        } else {
            return RenderPass::Type::Standard_Opaque;
        }
        break;
    case GeometryType::TreeMesh:
        if (transparent) {
            return RenderPass::Type::Tree_Transparent;
        } else {
            return RenderPass::Type::Tree_Opaque;
        }
        break;
    case GeometryType::PatchTerrain: return RenderPass::Type::Terrain;
    case GeometryType::WaterMesh: return RenderPass::Type::Water;
    case GeometryType::SkyMesh: return RenderPass::Type::Sky;
    default: return RenderPass::Type::Unknown;
    }
}

void Renderer::submit(Geometry* geom, const glm::mat4& model)
{
    if (!geom) return;
    if (geom->lods.empty()) return;
    if (!geom->uploaded) return;
    
    glm::vec3 cam_pos = _camera->getPosition();

    AABB world_aabb = geom->aabb.transform(model);
    
    float distance = distanceToAABB(cam_pos, world_aabb);

    size_t lod_index = 0;
    
    if (_lod_enabled && geom->type != GeometryType::SkyMesh)
        lod_index = selectLOD(distance, *geom);

    Geometry::LOD& lod = geom->lods[lod_index];

    const Frustum& frustum = _camera->getFrustum();

    if (_frustum_culling_enabled && geom->type != GeometryType::SkyMesh)
    {
        if (!frustum.intersects(world_aabb))
        {
            _stats.meshes_culled += lod.meshes.size();
            for (auto& mesh : lod.meshes)
                _stats.polygons_culled += mesh.index_count / 3;
            return;
        }
    }

    _context.transforms.push_back(model);

    for (auto& mesh : lod.meshes)
    {
        if (_frustum_culling_enabled)
        {
            if (geom->type == GeometryType::PatchTerrain || geom->type == GeometryType::WaterMesh)
            {
                AABB mesh_world_aabb = mesh.aabb.transform(model);
                if (!frustum.intersects(mesh_world_aabb))
                {
                    _stats.meshes_culled++;
                    _stats.polygons_culled += mesh.index_count / 3;
                    continue;
                }
            }
        }

        RenderCommand cmd;
        cmd.vao = lod.vao;
        cmd.index_count = mesh.index_count;
        cmd.index_offset = (void*)(mesh.index_start * sizeof(unsigned int));
        cmd.base_vertex = mesh.base_vertex;
        cmd.material = mesh.material;
        cmd.transform_id = (uint32_t)_context.transforms.size() - 1;
        cmd.distance_to_camera = distance;

        auto pass_type = selectPassType(geom->type, mesh.material && mesh.material->isTransparent());
        if (pass_type == RenderPass::Type::Unknown) continue;

        auto pass = getPass(pass_type);
        if (!pass) continue;

        switch (geom->type)
        {
        case GeometryType::PatchTerrain:
            cmd.textures[0] = _terrain_textures[0];
            cmd.textures[1] = _terrain_textures[1];
            break;
        case GeometryType::WaterMesh:
            cmd.textures[0] = _water_textures[0];
            cmd.textures[1] = _water_textures[1];
            break;
        default: break;
        }

        if (pass->add(cmd))
        {
            _stats.meshes_rendered++;
            _stats.polygons_rendered += mesh.index_count / 3;
        }
    }
}

void Renderer::flush()
{
    glClearColor(_clear_color.r, _clear_color.g, _clear_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    UBO_CameraBlock camera_data;
    camera_data.view = _camera->getViewMat();
    camera_data.projection = _camera->getProjMat();
    camera_data.view_pos = glm::vec4(_camera->getPosition(), 1.0f);

    glNamedBufferSubData(_camera_ubo, 0, sizeof(UBO_CameraBlock), &camera_data);

    if (_fog_dirty)
    {
        if (!RENDER_FOG) _fog.params.z = 0.0f;
        glNamedBufferSubData(_fog_ubo, 0, sizeof(UBO_FogBlock), &_fog);
        _fog_dirty = false;
    }

    if (_lighting_dirty)
    {
        glNamedBufferSubData(_lighting_ubo, 0, sizeof(UBO_LightingBlock), &_lighting);
        _lighting_dirty = false;
    }

    if (_water_dirty)
    {
        glNamedBufferSubData(_water_ubo, 0, sizeof(UBO_WaterBlock), &_water);
        _water_dirty = false;
    }

    glPolygonMode(GL_FRONT_AND_BACK, _context.wireframe_enabled ? GL_LINE : GL_FILL);

    for (auto& pass_type : _execution_order)
    {
        auto* pass = getPass(pass_type);
        if (!pass) continue;

        pass->execute(_context);
    }

    _context.transforms.clear();
}

void Renderer::registerCmds()
{   
    g_Console->bindProperty("Renderer.fogColor", g_Renderer, &Renderer::getFogColor, &Renderer::setFogColor);
    // g_Console->addAlias("Renderer.fogColorVec", "Renderer.fogColor");

    g_Console->bindProperty("Renderer.fogColorVec",
        [this] () { return getFogColor(); },
        [this] (const glm::vec3& color) {
            // Quantized D3DCOLOR / uint8_t logic from DICE 2002:
            // .con float -> [0..255] byte -> float.
            // 0.50 -> 127/255 -> 0.498039f.
            // Do not "fix" to precise floats, or fog clearing won't match original.
            glm::vec3 quantized = {
                (float)(uint8_t)(color.r * 255.0f) / 255.0f,
                (float)(uint8_t)(color.g * 255.0f) / 255.0f,
                (float)(uint8_t)(color.b * 255.0f) / 255.0f
            };
            setFogColor(quantized);
            _clear_color = quantized; // NOTE: Skybox is transparent
        }
    );
    
    g_Console->bindProperty("Renderer.fogStart", g_Renderer, &Renderer::getFogStart, &Renderer::setFogStart);
    g_Console->addAlias("Renderer.fogLinearStart", "Renderer.fogStart");
    
    g_Console->bindProperty("Renderer.fogEnd", g_Renderer, &Renderer::getFogEnd, &Renderer::setFogEnd);
    g_Console->addAlias("Renderer.fogLinearEnd", "Renderer.fogEnd");

    g_Console->bindProperty("Renderer.vertexFogEnable", g_Renderer, &Renderer::isFogEnabled, &Renderer::setFogEnabled);
    g_Console->bindProperty("Renderer.diffuseColor", g_Renderer, &Renderer::getDiffuseColor, &Renderer::setDiffuseColor);
    g_Console->bindProperty("Renderer.specularColor", g_Renderer, &Renderer::getSpecularColor, &Renderer::setSpecularColor);
    g_Console->bindProperty("Renderer.ambientColor", g_Renderer, &Renderer::getAmbientColor, &Renderer::setAmbientColor);
    g_Console->bindProperty("Renderer.globalAmbientColor", g_Renderer, &Renderer::getGlobalAmbientColor, &Renderer::setGlobalAmbientColor);
    g_Console->bindProperty("Renderer.sunDirection", g_Renderer, &Renderer::getSunDirection, &Renderer::setSunDirection);
    g_Console->bindProperty("Renderer.wireframe", g_Renderer, &Renderer::isWireframeEnabled, &Renderer::setWireframeEnabled);
    g_Console->bindProperty("Renderer.useFrustumCulling", g_Renderer, &Renderer::isFrustumCullingEnabled, &Renderer::setFrustumCullingEnabled);
    g_Console->bindProperty("Renderer.useLOD", g_Renderer, &Renderer::isLODEnabled, &Renderer::setLODEnabled);

    g_Console->registerCmd("Renderer.passEnabled", [this] (Console::ExecContext& ctx, const Console::CommandArgs& args) {
        if (args.empty()) return CommandResult{ "Not enough arguments! Usage: Renderer.passEnabled <pass_name> [enabled]" };
        
        auto pass_type = passTypeFromString(args[0]);
        if (pass_type == RenderPass::Type::Unknown)
            return CommandResult{ "Invalid pass name!", CommandStatus::Error };

        auto pass = getPass(pass_type);

        if (args.size() >= 2)
        {
            bool enabled = StringUtils::fromString<bool>(args[1]);
            pass->setEnabled(enabled);
        }

        return CommandResult { std::format("Pass \"{}\" is {}!", args[0], pass->isEnabled() ? "enabled" : "disabled")  };
    });

    g_Console->registerCmd("Renderer.listPasses", [this] (Console::ExecContext& ctx, const Console::CommandArgs& args) {
        std::string result_str = "";

        for (auto& pass : _passes)
        {
            if (!pass) continue;

            auto type_str = passTypeToString(pass->getType());
            result_str += std::format("{} | {}\n", type_str, pass->isEnabled() ? "enabled" : "disabled");
        }

        return CommandResult { result_str };
    });

    g_Console->bindProperty("Renderer.clearColor", g_Renderer, &Renderer::_clear_color);
}

void Renderer::resetStats()
{
    _stats.meshes_culled = 0;
    _stats.meshes_rendered = 0;
    _stats.polygons_culled = 0;
    _stats.polygons_rendered = 0;

    for (auto& pass : _passes)
        pass->clearStats();
}

RenderPass* Renderer::getPass(RenderPass::Type type) const
{
    return _passes[static_cast<size_t>(type)].get();
}

void Renderer::setViewport(int x, int y, int w, int h) const
{
    glViewport(x, y, w, h);
}
