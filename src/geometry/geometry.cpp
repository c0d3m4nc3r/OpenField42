#include "geometry/geometry.h"

#include "geometry/material.h"
#include "render/shader.h"
#include "utils/log.h"

#include "glad/gl.h"

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
