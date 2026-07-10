#include "render/renderer.h"
#include "render/camera.h"
#include "core/config.h"
#include "render/render_passes.h"
#include "render/shader.h"
#include "world/sky.h"

#include "glad/gl.h"

#include <SDL3/SDL_timer.h>

bool Renderer::init()
{
    LOG_INFO("Renderer::init: Initializing renderer...");

    // Load standard shader
    _shaders.standard = std::make_unique<Shader>();
    if (!_shaders.standard->load("shaders/standard.vert", "shaders/standard.frag"))
    {
        LOG_ERROR("Renderer::init: Failed to load standard shader!");
        return false;
    }

    // Load sky shader
    _shaders.sky = std::make_unique<Shader>();
    if (!_shaders.sky->load("shaders/sky.vert", "shaders/sky.frag"))
    {
        LOG_ERROR("Renderer::init: Failed to load sky shader!");
        return false;
    }

    // Load water shader
    _shaders.water = std::make_unique<Shader>();
    if (!_shaders.water->load("shaders/water.vert", "shaders/water.frag"))
    {
        LOG_ERROR("Renderer::init: Failed to load water shader!");
        return false;
    }

    // Load terrain shader
    _shaders.terrain = std::make_unique<Shader>();
    if (!_shaders.terrain->load("shaders/terrain.vert", "shaders/terrain.frag"))
    {
        LOG_ERROR("Renderer::init: Failed to load terrain shader!");
        return false;
    }

    // Setup Camera UBO
    glGenBuffers(1, &_camera_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _camera_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_CameraBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint camera_block_index = glGetUniformBlockIndex(_shaders.standard->getID(), "CameraBlock");
    glUniformBlockBinding(_shaders.standard->getID(), camera_block_index, 0);
    
    camera_block_index = glGetUniformBlockIndex(_shaders.sky->getID(), "CameraBlock");
    glUniformBlockBinding(_shaders.sky->getID(), camera_block_index, 0);

    camera_block_index = glGetUniformBlockIndex(_shaders.water->getID(), "CameraBlock");
    glUniformBlockBinding(_shaders.water->getID(), camera_block_index, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, _camera_ubo);

    // Setup Fog UBO
    glGenBuffers(1, &_fog_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _fog_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_FogBlock), &_fog, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint fog_block_index = glGetUniformBlockIndex(_shaders.standard->getID(), "FogBlock");
    glUniformBlockBinding(_shaders.standard->getID(), fog_block_index, 1);

    fog_block_index = glGetUniformBlockIndex(_shaders.water->getID(), "FogBlock");
    glUniformBlockBinding(_shaders.water->getID(), fog_block_index, 1);

    fog_block_index = glGetUniformBlockIndex(_shaders.terrain->getID(), "FogBlock");
    glUniformBlockBinding(_shaders.terrain->getID(), fog_block_index, 1);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, _fog_ubo);

    // Setup Lighting UBO
    glGenBuffers(1, &_lighting_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _lighting_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_LightingBlock), &_lighting, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint lighting_block_index = glGetUniformBlockIndex(_shaders.standard->getID(), "LightingBlock");
    glUniformBlockBinding(_shaders.standard->getID(), lighting_block_index, 2);

    lighting_block_index = glGetUniformBlockIndex(_shaders.terrain->getID(), "LightingBlock");
    glUniformBlockBinding(_shaders.terrain->getID(), lighting_block_index, 2);

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, _lighting_ubo);

    // Setup Water UBO
    glGenBuffers(1, &_water_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _water_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_WaterBlock), &_water, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint water_block_index = glGetUniformBlockIndex(_shaders.water->getID(), "WaterBlock");
    glUniformBlockBinding(_shaders.water->getID(), water_block_index, 3);

    glBindBufferBase(GL_UNIFORM_BUFFER, 3, _water_ubo);

    // Create passes

    _passes[static_cast<size_t>(RenderPass::Type::Terrain)] = std::make_unique<TerrainPass>(_shaders.terrain.get());
    _passes[static_cast<size_t>(RenderPass::Type::Opaque)] = std::make_unique<OpaquePass>(_shaders.standard.get());
    _passes[static_cast<size_t>(RenderPass::Type::Transparent)] = std::make_unique<TransparentPass>(_shaders.standard.get());
    _passes[static_cast<size_t>(RenderPass::Type::Water)] = std::make_unique<WaterPass>(_shaders.water.get());
    _passes[static_cast<size_t>(RenderPass::Type::Sky)] = std::make_unique<SkyPass>(_shaders.sky.get());

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

    _shaders.terrain.reset();
    _shaders.standard.reset();
    _shaders.sky.reset();
    _shaders.water.reset();

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
    const Frustum& frustum = _camera->getFrustum();

    glm::vec3 world_center = glm::vec3(model * glm::vec4(geom->aabb.center(), 1.0f));
    float distance = glm::length(world_center - cam_pos);

    size_t lod_index = 0;
    
    if (USE_LODS && geom->type != GeometryType::SkyMesh)
    {
        // TODO: Load LOD max distance from .con files
        lod_index = calculateLOD(distance, _camera->getFarPlane()*0.9f, geom->lods.size() - 1);
    }

    Geometry::LOD& lod = geom->lods[lod_index];

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

        if (geom->type == GeometryType::WaterMesh) {
            cmd.textures[0] = _water_textures[0];
            cmd.textures[1] = _water_textures[1];
            getPass(RenderPass::Type::Water)->add(cmd);
        } else if (geom->type == GeometryType::PatchTerrain) {
            cmd.textures[0] = _terrain_textures[0];
            cmd.textures[1] = _terrain_textures[1];
            getPass(RenderPass::Type::Terrain)->add(cmd);
        } else if (geom->type == GeometryType::SkyMesh) {
            getPass(RenderPass::Type::Sky)->add(cmd);
        } else if (mesh.material && mesh.material->transparent) {
            getPass(RenderPass::Type::Transparent)->add(cmd);    
        } else {
            getPass(RenderPass::Type::Opaque)->add(cmd);
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

    glBindBuffer(GL_UNIFORM_BUFFER, _camera_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO_CameraBlock), &camera_data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if (_fog_dirty)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, _fog_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO_FogBlock), &_fog);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        _fog_dirty = false;
    }

    if (_lighting_dirty)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, _lighting_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO_LightingBlock), &_lighting);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        _lighting_dirty = false;
    }

    if (_water_dirty)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, _water_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO_WaterBlock), &_water);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        _water_dirty = false;
    }

    glPolygonMode(GL_FRONT_AND_BACK, _context.wireframe_enabled ? GL_LINE : GL_FILL);

    for (auto& pass_type : _execution_order)
    {
        auto* pass = getPass(pass_type);
        if (!pass) continue;

        pass->execute(_context);
    }
}

void Renderer::reloadShaders()
{
    LOG_INFO("Renderer::reloadShaders: Reloading all shaders...");

    auto new_standard = std::make_unique<Shader>();
    auto new_sky = std::make_unique<Shader>();
    auto new_water = std::make_unique<Shader>();
    auto new_terrain = std::make_unique<Shader>();

    bool success = true;

    if (!new_standard->load("shaders/standard.vert", "shaders/standard.frag"))
    {
        LOG_ERROR("Renderer::reloadShaders: Failed to load NEW standard shader!");
        success = false;
    }

    if (!new_sky->load("shaders/sky.vert", "shaders/sky.frag"))
    {
        LOG_ERROR("Renderer::reloadShaders: Failed to load NEW sky shader!");
        success = false;
    }

    if (!new_water->load("shaders/water.vert", "shaders/water.frag"))
    {
        LOG_ERROR("Renderer::reloadShaders: Failed to load NEW water shader!");
        success = false;
    }

    if (!new_terrain->load("shaders/terrain.vert", "shaders/terrain.frag"))
    {
        LOG_ERROR("Renderer::reloadShaders: Failed to load NEW terrain shader!");
        success = false;
    }

    if (!success)
    {
        LOG_ERROR("Renderer::reloadShaders: Shader reload failed! Retaining old shaders.");
        return;
    }

    _shaders.standard = std::move(new_standard);
    _shaders.sky = std::move(new_sky);
    _shaders.water = std::move(new_water);
    _shaders.terrain = std::move(new_terrain);

    auto bindUniformBlocks = [](GLuint program_id) {
        GLuint index = glGetUniformBlockIndex(program_id, "CameraBlock");
        if (index != GL_INVALID_INDEX) glUniformBlockBinding(program_id, index, 0);

        index = glGetUniformBlockIndex(program_id, "FogBlock");
        if (index != GL_INVALID_INDEX) glUniformBlockBinding(program_id, index, 1);

        index = glGetUniformBlockIndex(program_id, "LightingBlock");
        if (index != GL_INVALID_INDEX) glUniformBlockBinding(program_id, index, 2);

        index = glGetUniformBlockIndex(program_id, "WaterBlock");
        if (index != GL_INVALID_INDEX) glUniformBlockBinding(program_id, index, 3);
    };

    bindUniformBlocks(_shaders.standard->getID());
    bindUniformBlocks(_shaders.sky->getID());
    bindUniformBlocks(_shaders.water->getID());
    bindUniformBlocks(_shaders.terrain->getID());

    _passes[static_cast<size_t>(RenderPass::Type::Terrain)]->setShader(_shaders.terrain.get());
    _passes[static_cast<size_t>(RenderPass::Type::Opaque)]->setShader(_shaders.standard.get());
    _passes[static_cast<size_t>(RenderPass::Type::Transparent)]->setShader(_shaders.standard.get());
    _passes[static_cast<size_t>(RenderPass::Type::Water)]->setShader(_shaders.water.get());
    _passes[static_cast<size_t>(RenderPass::Type::Sky)]->setShader(_shaders.sky.get());

    LOG_INFO("Renderer::reloadShaders: All shaders reloaded and re-bound successfully!");
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
