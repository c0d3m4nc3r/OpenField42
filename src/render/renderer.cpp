#include "render/renderer.h"

#include "core/globals.h"
#include "core/config.h"
#include "render/camera.h"
#include "render/render_passes.h"
#include "render/shader.h"
#include "render/shader_manager.h"
#include "world/sky.h"

#include "glad/gl.h"

#include <SDL3/SDL_timer.h>

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

static size_t calculateLOD(float distance, float max_distance, size_t max_lod)
{
    if (distance >= max_distance) 
        return max_lod;
    
    float ratio = distance / max_distance;
    size_t lod = static_cast<size_t>(ratio * max_lod);
    return glm::min(lod, max_lod);
}

void Renderer::submit(Geometry* geom, const glm::mat4& model)
{
    if (!geom) return;
    if (geom->lods.empty()) return;

    glm::vec3 cam_pos = _camera->getPosition();
    glm::vec3 cam_forward = _camera->getForward(); 

    glm::vec3 world_center = glm::vec3(model * glm::vec4(geom->aabb.center(), 1.0f));

    glm::vec3 to_object = world_center - cam_pos;

    float distance = glm::dot(to_object, cam_forward);

    size_t lod_index = 0;
    
    if (USE_LODS && geom->type != GeometryType::SkyMesh)
    {
        // TODO: Load LOD max distance from .con files
        lod_index = calculateLOD(distance, _camera->getFarPlane()*0.9f, geom->lods.size() - 1);
    }

    Geometry::LOD& lod = geom->lods[lod_index];

    const Frustum& frustum = _camera->getFrustum();

    if (USE_FRUSTUM_CULLING && geom->type != GeometryType::SkyMesh)
    {
        AABB world_aabb = geom->aabb.transform(model);
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
        if (USE_FRUSTUM_CULLING)
        {
            if (geom->type == GeometryType::PatchTerrain || geom->type == GeometryType::WaterMesh)
            {
                AABB world_aabb = mesh.aabb.transform(model);
                if (!frustum.intersects(world_aabb))
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

        switch (geom->type)
        {
        case GeometryType::StandardMesh:
            if (mesh.material && mesh.material->isTransparent()) {
                getPass(RenderPass::Type::Standard_Transparent)->add(cmd);
            } else {
                getPass(RenderPass::Type::Standard_Opaque)->add(cmd);
            }
            break;
        case GeometryType::TreeMesh:
            if (mesh.material && mesh.material->isTransparent()) {
                getPass(RenderPass::Type::Tree_Transparent)->add(cmd);
            } else {
                getPass(RenderPass::Type::Tree_Opaque)->add(cmd);
            }
            break;
        case GeometryType::PatchTerrain:
            cmd.textures[0] = _terrain_textures[0];
            cmd.textures[1] = _terrain_textures[1];
            getPass(RenderPass::Type::Terrain)->add(cmd);
            break;
        case GeometryType::WaterMesh:
            cmd.textures[0] = _water_textures[0];
            cmd.textures[1] = _water_textures[1];
            getPass(RenderPass::Type::Water)->add(cmd);
            break;
        case GeometryType::SkyMesh:
            getPass(RenderPass::Type::Sky)->add(cmd);
            break;
        default:
            continue;
        }

        _stats.meshes_rendered++;
        _stats.polygons_rendered += mesh.index_count / 3;
    }
}

void Renderer::flush()
{
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

void Renderer::resetStats()
{
    _stats.meshes_culled = 0;
    _stats.meshes_rendered = 0;
    _stats.polygons_culled = 0;
    _stats.polygons_rendered = 0;
}

void Renderer::setViewport(int x, int y, int w, int h) const
{
    glViewport(x, y, w, h);
}

RenderPass* Renderer::getPass(RenderPass::Type type)
{
    return _passes[static_cast<size_t>(type)].get();
}
