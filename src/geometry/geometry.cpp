#include "geometry/geometry.h"

#include "geometry/material.h"
#include "render/shader.h"
#include "utils/log.h"

#include "glad/gl.h"

bool Geometry::LOD::upload()
{
    if (uploaded)
    {
        LOG_WARNING("Geometry::LOD::upload: Mesh is already uploaded!");
        return true;
    }

    if (vertices.empty() || indices.empty())
    {
        // LOG_ERROR("Geometry::LOD::upload: Vertices or indices are empty!");
        return true;
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

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, sprite_ofs));

    glBindVertexArray(0);

    vertices.clear();
    indices.clear();

    uploaded = true;

    return true;
}

void Geometry::LOD::unload()
{
    if (!uploaded) return;
    
    if (vao) glDeleteVertexArrays(1, &vao);
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);

    vao = 0;
    vbo = 0;
    ebo = 0;
    
    uploaded = false;
}

void Geometry::LOD::draw(Shader* shader) const
{
    if (!shader || !vao || !uploaded || meshes.empty()) return;

    glBindVertexArray(vao);

    for (auto& mesh : meshes)
    {
        if (mesh.material)
            mesh.material->apply(shader);

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            mesh.index_count,
            GL_UNSIGNED_INT,
            (void*)(mesh.index_start * sizeof(unsigned int)),
            mesh.base_vertex
        );
    }
}

Geometry::~Geometry()
{
    unload();
}

void Geometry::draw(Shader* shader, const glm::mat4& model, int lod_level) const
{
    if (lods.empty() || !shader) return;

    shader->setMat4("uModel", model);

    const LOD& lod = lods[lod_level];
    lod.draw(shader);
}

bool Geometry::upload()
{
    for (size_t i = 0; i < lods.size(); i++)
    {
        LOD& lod = lods[i];
        if (lod.uploaded) continue;

        if (!lod.upload())
        {
            LOG_ERROR("Geometry::upload: Failed to upload lod %zu!", i);
            return false;
        }
    }

    return true;
}

void Geometry::unload()
{
    for (auto& lod : lods)
    {
        if (!lod.uploaded) continue;
        lod.unload();
    }
}

