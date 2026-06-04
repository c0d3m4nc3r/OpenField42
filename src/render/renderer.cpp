#include "render/renderer.h"
#include "render/camera.h"
#include "core/config.h"
#include "core/game.h"
#include "render/shader.h"
#include "world/sky.h"
#include "world/water.h"

#include "glad/glad.h"

#include <SDL3/SDL_timer.h>

#include <algorithm>

Renderer g_Renderer;

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
    
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, _fog_ubo);

    // Setup Lighting UBO
    glGenBuffers(1, &_lighting_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _lighting_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_LightingBlock), &_lighting, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GLuint lighting_block_index = glGetUniformBlockIndex(_shaders.standard->getID(), "LightingBlock");
    glUniformBlockBinding(_shaders.standard->getID(), lighting_block_index, 2);
    
    lighting_block_index = glGetUniformBlockIndex(_shaders.water->getID(), "LightingBlock");
    glUniformBlockBinding(_shaders.water->getID(), lighting_block_index, 2);

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, _lighting_ubo);

    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    LOG_INFO("Renderer::init: Renderer initialized!");

    return true;
}

void Renderer::shutdown()
{
    LOG_INFO("Renderer::shutdown: Shutting down renderer...");

    if (_camera_ubo) glDeleteBuffers(1, &_camera_ubo);
    if (_fog_ubo) glDeleteBuffers(1, &_fog_ubo);
    if (_lighting_ubo) glDeleteBuffers(1, &_lighting_ubo);

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
    
    if (USE_LODS && geom->type != GeometryType::PatchTerrain)
    {
        // TODO: Load LOD max distance from .con files
        lod_index = calculateLOD(distance, g_Game.view_distance*0.9f, geom->lods.size() - 1);
    }

    Geometry::LOD& lod = geom->lods[lod_index];

    if (USE_FRUSTUM_CULLING)
    {
        AABB world_aabb = geom->aabb.transform(model);
        if (!frustum.intersects(world_aabb))
        {
            _stats.meshes_culled += lod.meshes.size();
            for (auto& mesh : lod.meshes)
                _stats.polygons_culled += mesh.poly_count;
            return;
        }
    }

    for (auto& mesh : lod.meshes)
    {
        if (!mesh.material) continue;

        if (USE_FRUSTUM_CULLING && !mesh.use_geom_aabb)
        {
            AABB world_aabb = mesh.aabb.transform(model);
            if (!frustum.intersects(world_aabb))
            {
                _stats.meshes_culled++;
                _stats.polygons_culled += mesh.poly_count;
                continue;
            }
        }

        RenderItem item;
        item.mesh = &mesh;
        item.model = &model;
        item.geom = geom;
        item.distance_to_camera = distance;

        if (mesh.material->transparent)
            _transparent_queue.push_back(item);
        else
            _opaque_queue.push_back(item);

        _stats.meshes_rendered++;
        _stats.polygons_rendered += mesh.poly_count;
    }
}

void Renderer::flush()
{
    UBO_CameraBlock camera_data;
    camera_data.view = _camera->getViewMat();
    camera_data.projection = _camera->getProjMat();
    camera_data.view_pos = _camera->getPosition();

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
    
    opaquePass();
    skyPass();
    waterPass();
    transparentPass();

    _opaque_queue.clear();
    _transparent_queue.clear();
}

void Renderer::resetStats()
{
    _stats.meshes_culled = 0;
    _stats.meshes_rendered = 0;
    _stats.polygons_culled = 0;
    _stats.polygons_rendered = 0;
}

void Renderer::opaquePass()
{
    Shader* shader = _shaders.standard.get();

    shader->use();

    shader->setFloat("uTime", (float)SDL_GetTicks() / 1000.0f);
    shader->setVec3("uSunLightDir", g_Sky.sun_light_dir);

    shader->setBool("uWireframeMode", wireframe_mode);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

    for (auto& item : _opaque_queue)
    {
        _shaders.standard->setMat4("uModel", *item.model);
        if (item.geom->type == GeometryType::StandardMesh)
            _shaders.standard->setVec3("uWireframeColor", glm::vec3(1.0f, 0.0f, 0.0f));
        else if (item.geom->type == GeometryType::PatchTerrain)
            _shaders.standard->setVec3("uWireframeColor", glm::vec3(0.0f, 1.0f, 0.0f));
        item.mesh->draw(_shaders.standard.get());
    }
}

void Renderer::transparentPass()
{
    Shader* shader = _shaders.standard.get();

    shader->use();

    std::sort(_transparent_queue.begin(), _transparent_queue.end(), 
        [](const RenderItem& a, const RenderItem& b) {
            return a.distance_to_camera > b.distance_to_camera;
        });

    _shaders.standard->setVec3("uWireframeColor", glm::vec3(1.0f, 0.0f, 0.0f));

    for (auto& item : _transparent_queue)
    {
        _shaders.standard->setMat4("uModel", *item.model);
        item.mesh->draw(_shaders.standard.get());
    }
}

void Renderer::skyPass()
{
    Shader* shader = _shaders.sky.get();

    shader->use();
    shader->setBool("uWireframeMode", wireframe_mode);

    g_Sky.draw(shader);
}

void Renderer::waterPass()
{
    Shader* shader = _shaders.water.get();

    shader->use();
    shader->setFloat("uTime", (float)SDL_GetTicks() / 1000.0f);
    shader->setVec3("uSunLightDir", g_Sky.sun_light_dir);

    shader->setBool("uWireframeMode", wireframe_mode);
    
    g_Water.draw(shader);
}
