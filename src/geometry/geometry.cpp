#include "geometry/geometry.h"

#include "geometry/template.h"
#include "geometry/material.h"
#include "geometry/standard_mesh.h"
#include "render/shader.h"
#include "utils/log.h"
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
    unload();
}

void Geometry::Mesh::draw(Shader* shader) const
{
    if (!vao || !shader || !uploaded) return;

    if (material)
        material->apply(shader);

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

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);

    index_count = indices.size();
    poly_count = index_count / 3;
    vertices.clear();
    indices.clear();

    uploaded = true;

    return true;
}

void Geometry::Mesh::unload()
{
    if (!uploaded) return;
    
    if (vao) glDeleteVertexArrays(1, &vao);
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);

    vao = 0;
    vbo = 0;
    ebo = 0;
    index_count = 0;
    poly_count = 0;
    uploaded = false;
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

    if (tmpl->type != GeometryType::StandardMesh)
    {
        LOG_ERROR("Geometry::create: Unsupported geometry type: %s!",
            geometryTypeToString(tmpl->type).c_str());
        return nullptr;
    }

    StandardMesh* stdmesh_geom = new StandardMesh();
    geom = stdmesh_geom;
    if (!stdmesh_geom->load(tmpl))
    {
        LOG_ERROR("Geometry::create: Failed to load StandardMesh geometry from template '%s'!", tmpl->name.c_str());
        delete stdmesh_geom;
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

void Geometry::draw(Shader* shader, const glm::mat4& model) const
{
    if (lods.empty() || !shader) return;

    shader->setMat4("uModel", model);

    const LOD& current_lod = lods[0];
    for (const auto& mesh : current_lod.meshes)
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
