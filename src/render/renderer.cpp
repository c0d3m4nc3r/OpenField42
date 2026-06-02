#include "render/renderer.h"
#include "render/camera.h"
#include "core/config.h"
#include "core/game.h"
#include "world/sky.h"
#include "render/shader.h"

#include "glad/glad.h"

#include <SDL3/SDL_timer.h>

#include <algorithm>

Renderer g_Renderer;

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

        if (mesh.material->transparent || mesh.is_water)
            _transparent_queue.push_back(item);
        else
            _opaque_queue.push_back(item);

        _stats.meshes_rendered++;
        _stats.polygons_rendered += mesh.poly_count;
    }
}

void Renderer::flush()
{
    if (!_shader || !_camera) return;

    _shader->bind();

    _shader->setMat4("uView", _camera->getViewMat());
    _shader->setMat4("uProjection", _camera->getProjMat());
    _shader->setVec3("uViewPos", _camera->getPosition());
    _shader->setFloat("uTime", (float)SDL_GetTicks() / 1000.0f);

    if (fog.enabled && RENDER_FOG)
    {
        _shader->setVec3("uFog.color", fog.color.toVec3());
        _shader->setFloat("uFog.start", fog.start);
        _shader->setFloat("uFog.end", fog.end);
        _shader->setBool("uFog.enabled", fog.enabled);
    } else {
        _shader->setBool("uFog.enabled", false);
    }

    _shader->setVec3("uLighting.diffuse", lighting.diffuse.toVec3());
    _shader->setVec3("uLighting.specular", lighting.specular.toVec3());
    _shader->setVec3("uLighting.ambient", lighting.ambient.toVec3());
    _shader->setVec3("uLighting.globalAmbient", lighting.global_ambient.toVec3());

    _shader->setVec3("uSunLightDir", g_Sky.sun_light_dir);

    _shader->setBool("uWireframeMode", wireframe_mode);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

    // Sort transparent objects by distance to camera
    std::sort(_transparent_queue.begin(), _transparent_queue.end(), 
        [](const RenderItem& a, const RenderItem& b) {
            return a.distance_to_camera > b.distance_to_camera;
        });

    auto renderItems = [this](auto& queue)
    {
        for (auto& item : queue)
        {
            _shader->setMat4("uModel", *item.model);
            if (item.geom->type == GeometryType::StandardMesh)
                _shader->setVec3("uWireframeColor", glm::vec3(1.0f, 0.0f, 0.0f));
            else if (item.geom->type == GeometryType::PatchTerrain)
                _shader->setVec3("uWireframeColor", glm::vec3(0.0f, 1.0f, 0.0f));
            item.mesh->draw(_shader.get());
        }
        queue.clear();
    };

    renderItems(_opaque_queue);

    g_Sky.draw(_shader.get());
    
    renderItems(_transparent_queue);

    _shader->unbind();
}

void Renderer::resetStats()
{
    _stats.meshes_culled = 0;
    _stats.meshes_rendered = 0;
    _stats.polygons_culled = 0;
    _stats.polygons_rendered = 0;
}

const Renderer::Stats& Renderer::getStats() const
{
    return _stats;
}

void Renderer::setCamera(Camera* camera) { this->_camera = camera; }
void Renderer::setShader(std::shared_ptr<Shader> shader) { this->_shader = shader; }