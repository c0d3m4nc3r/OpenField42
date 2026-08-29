#include "geometry/geometry.h"

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

    glCreateVertexArrays(1, &vao);
    glCreateBuffers(1, &vbo);
    glCreateBuffers(1, &ebo);

    glNamedBufferData(vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glNamedBufferData(ebo, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexArrayElementBuffer(vao, ebo);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));

    glEnableVertexArrayAttrib(vao, 0);
    glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
    glVertexArrayAttribBinding(vao, 0, 0);

    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
    glVertexArrayAttribBinding(vao, 1, 0);

    glEnableVertexArrayAttrib(vao, 2);
    glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
    glVertexArrayAttribBinding(vao, 2, 0);

    glEnableVertexArrayAttrib(vao, 3);
    glVertexArrayAttribFormat(vao, 3, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
    glVertexArrayAttribBinding(vao, 3, 0);

    glEnableVertexArrayAttrib(vao, 4);
    glVertexArrayAttribFormat(vao, 4, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, sprite_ofs));
    glVertexArrayAttribBinding(vao, 4, 0);

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

Geometry::~Geometry()
{
    unload();
}

bool Geometry::upload()
{
    if (uploaded) return true;

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

    uploaded = true;

    return true;
}

void Geometry::unload()
{
    if (!uploaded) return;

    for (auto& lod : lods)
    {
        if (!lod.uploaded) continue;
        lod.unload();
    }

    uploaded = false;
}