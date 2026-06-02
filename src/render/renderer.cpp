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

    glm::vec3 cam_pos = camera->getPosition();
    const Frustum& frustum = camera->getFrustum();

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
            stats.meshes_culled += lod.meshes.size();
            for (auto& mesh : lod.meshes)
                stats.polygons_culled += mesh.poly_count;
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
                stats.meshes_culled++;
                stats.polygons_culled += mesh.poly_count;
                continue;
            }
        }

        RenderItem item;
        item.mesh = &mesh;
        item.model = &model;
        item.geom = geom;
        item.distance_to_camera = distance;

        if (mesh.material->transparent || mesh.is_water)
            transparent_queue.push_back(item);
        else
            opaque_queue.push_back(item);

        stats.meshes_rendered++;
        stats.polygons_rendered += mesh.poly_count;
    }
}

void Renderer::flush()
{
    if (!shader || !camera) return;

    shader->bind();

    shader->setMat4("uView", camera->getViewMat());
    shader->setMat4("uProjection", camera->getProjMat());
    shader->setVec3("uViewPos", camera->getPosition());
    shader->setFloat("uTime", (float)SDL_GetTicks() / 1000.0f);

    if (fog.enabled && RENDER_FOG)
    {
        shader->setVec3("uFog.color", fog.color.toVec3());
        shader->setFloat("uFog.start", fog.start);
        shader->setFloat("uFog.end", fog.end);
        shader->setBool("uFog.enabled", fog.enabled);
    } else {
        shader->setBool("uFog.enabled", false);
    }

    shader->setVec3("uLighting.diffuse", lighting.diffuse.toVec3());
    shader->setVec3("uLighting.specular", lighting.specular.toVec3());
    shader->setVec3("uLighting.ambient", lighting.ambient.toVec3());
    shader->setVec3("uLighting.globalAmbient", lighting.global_ambient.toVec3());

    shader->setVec3("uSunLightDir", sky.sun_light_dir);

    shader->setBool("uWireframeMode", wireframe_mode);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

    // Sort transparent objects by distance to camera
    std::sort(transparent_queue.begin(), transparent_queue.end(), 
        [](const RenderItem& a, const RenderItem& b) {
            return a.distance_to_camera > b.distance_to_camera;
        });

    auto renderItems = [this](auto& queue)
    {
        for (auto& item : queue)
        {
            shader->setMat4("uModel", *item.model);
            if (item.geom->type == GeometryType::StandardMesh)
                shader->setVec3("uWireframeColor", glm::vec3(1.0f, 0.0f, 0.0f));
            else if (item.geom->type == GeometryType::PatchTerrain)
                shader->setVec3("uWireframeColor", glm::vec3(0.0f, 1.0f, 0.0f));
            item.mesh->draw(shader.get());
        }
        queue.clear();
    };

    renderItems(opaque_queue);

    sky.draw(shader.get());
    
    renderItems(transparent_queue);

    shader->unbind();
}

void Renderer::resetStats()
{
    stats.meshes_culled = 0;
    stats.meshes_rendered = 0;
    stats.polygons_culled = 0;
    stats.polygons_rendered = 0;
}

const Renderer::Stats& Renderer::getStats() const
{
    return stats;
}

void Renderer::setCamera(Camera* camera) { this->camera = camera; }
void Renderer::setShader(std::shared_ptr<Shader> shader) { this->shader = shader; }