#include "geometry/geometry.h"
#include "render/texture.h"
#include "geometry/template.h"
#include "geometry/material.h"
#include "geometry/standard_mesh.h"
#include "geometry/patch_terrain.h"
#include "utils/log.h"
#include "world/water.h"
#include "render/shader.h"
#include "utils/string_utils.h"

#include "glad/glad.h"

std::string geometryTypeToString(GeometryType type)
{
    switch (type)
    {
    case GeometryType::AnimatedMesh: return "AnimatedMesh";
    case GeometryType::SkeletonCollisionMesh: return "SkeletonCollisionMesh";
    case GeometryType::StandardMesh: return "StandardMesh";
    case GeometryType::TreeMesh: return "TreeMesh";
    case GeometryType::PatchTerrain: return "PatchTerrain";
    case GeometryType::Unknown:
    default: return "Unknown";
    }
}

GeometryType geometryTypeFromString(const std::string& str)
{   
    static const std::unordered_map<std::string, GeometryType> lut = {
        {"animatedmesh", GeometryType::AnimatedMesh},
        {"skeletoncollisionmesh", GeometryType::SkeletonCollisionMesh},
        {"standardmesh", GeometryType::StandardMesh},
        {"treemesh", GeometryType::TreeMesh},
        {"patchterrain", GeometryType::PatchTerrain}
    };

    auto it = lut.find(StringUtils::lowercase(str));
    if (it != lut.end())
        return it->second;

    return GeometryType::Unknown;
}

Geometry::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : vertices(vertices), indices(indices), vao(0), vbo(0), ebo(0) {} 

Geometry::Mesh::~Mesh()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
}

void Geometry::Mesh::draw(Shader* shader)
{
    if (!vao || !shader || !uploaded) return;

    if (material)
        material->apply(shader);

    shader->setBool("uIsWater", is_water);

    if (is_water)
    {
        if (g_Water.layers[0].texture)
        {
            g_Water.layers[0].texture->bind(0);
            shader->setInt("uWater.texLayer1", 0);
            shader->setVec2("uWater.scrollDir1", g_Water.layers[0].scroll_dir);
            shader->setFloat("uWater.scrollSpeed1", g_Water.layers[0].scroll_speed);
        }

        if (g_Water.layers[1].texture)
        {
            g_Water.layers[1].texture->bind(1);
            shader->setInt("uWater.texLayer2", 1);
            shader->setVec2("uWater.scrollDir2", g_Water.layers[1].scroll_dir);
            shader->setFloat("uWater.scrollSpeed2", g_Water.layers[1].scroll_speed);
        }

        if (g_Water.depth_map)
        {
            g_Water.depth_map->bind(2);
            shader->setInt("uWater.depthMap", 2);
        }

        shader->setFloat("uWater.minDepth", g_Water.min_depth);
        shader->setFloat("uWater.maxDepth", g_Water.max_depth);
        
        shader->setVec4("uWater.shallowColor", g_Water.shallow_color.toVec4());
        shader->setVec4("uWater.deepColor", g_Water.deep_color.toVec4());
        shader->setFloat("uWater.alphaDepth", g_Water.alpha_depth);


        shader->setVec3("uWireframeColor", glm::vec3(0.0f, 0.0f, 1.0f));
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool Geometry::Mesh::upload()
{
    if (uploaded)
    {
        LOG_WARNING("Geometry::Mesh::upload: Mesh is already uploaded!");
        return true;
    }

    if (vertices.empty() || indices.empty())
    {
        LOG_ERROR("Geometry::Mesh::upload: Vertices or indices are empty!");
        return false;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glBindVertexArray(0);

    index_count = indices.size();
    poly_count = index_count / 3;
    vertices.clear();
    indices.clear();

    uploaded = true;

    return true;
}

Geometry* Geometry::create(const GeometryTemplate* tmpl)
{
    if (!tmpl)
    {
        LOG_ERROR("Geometry::create: Template is NULL!");
        return nullptr;
    }

    auto it = cache.find(tmpl->name);
    if (it != cache.end())
        return it->second.get();

    Geometry* geom = nullptr;

    switch (tmpl->type)
    {
    case GeometryType::StandardMesh:
        {
            StandardMesh* mesh = new StandardMesh();
            geom = mesh;
            if (!mesh->load(tmpl))
            {
                LOG_ERROR("Geometry::create: Failed to load StandardMesh geometry from template '%s'!", tmpl->name.c_str());
                delete mesh;
                return nullptr;
            }
        } break;
        case GeometryType::PatchTerrain:
        {
            PatchTerrain* terrain = new PatchTerrain();
            geom = terrain;
            if (!terrain->load(tmpl))
            {
                LOG_ERROR("Geometry::create: Failed to load PatchTerrain geometry from template '%s'!", tmpl->name.c_str());
                delete terrain;
                return nullptr;
            }
        } break;
    default:
        LOG_ERROR("Geometry::create: Unsupported geometry type: %s!",
            geometryTypeToString(tmpl->type).c_str());
        return nullptr;
    }

    geom->type = tmpl->type;

    cache[tmpl->name] = std::unique_ptr<Geometry>(geom);

    return geom;
}

bool Geometry::uploadAll()
{
    LOG_INFO("Geometry::uploadAll: Uploading %zu geometries to GPU...", cache.size());

    for (auto& [name, geometry] : cache)
    {
        if (!geometry->upload())
        {
            LOG_ERROR("Geometry::uploadAll: Failed to upload geometry '%s' to GPU!", name.c_str());
            return false;
        }
    }

    LOG_INFO("Geometry::uploadAll: All geometries uploaded to GPU successfully!");

    return true;
}

void Geometry::draw(Shader* shader, const glm::mat4& model)
{
    if (lods.empty() || !shader) return;

    shader->setMat4("uModel", model);

    LOD& current_lod = lods[0];
    for (auto& mesh : current_lod.meshes)
        mesh.draw(shader);
}

bool Geometry::upload()
{
    for (size_t i = 0; i < lods.size(); i++)
    {
        LOD& lod = lods[i];
        for (size_t j = 0; j < lod.meshes.size(); j++)
        {
            Mesh& mesh = lod.meshes[j];
            if (mesh.uploaded) continue;

            if (!mesh.upload())
            {
                LOG_ERROR("Geometry::upload: Failed to upload mesh %d of lod %d!", j, i);
                return false;
            }
        }
    }

    return true;
}
