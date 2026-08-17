#pragma once

#include "geometry/geometry_type.h"
#include "geometry/material.h"

#include "math/aabb.h"

#include <vector>
#include <unordered_map>
#include <string>

struct GeometryTemplate;

class Terrain;
class Shader;
class Geometry
{
public:

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec4 color;
        glm::vec2 sprite_ofs;
    };

    struct Mesh
    {
        uint32_t index_count = 0;
        uint32_t index_start = 0;
        int base_vertex = 0;

        AABB aabb;
        Material* material = nullptr;
    };

    struct LOD
    {
        std::vector<Mesh> meshes;
        
        uint32_t vao = 0, vbo = 0, ebo = 0;
        bool uploaded = false;
        
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        
        float distance = 0.0f;

        bool upload();
        void unload();
    };

    virtual ~Geometry();

    GeometryType type = GeometryType::Unknown;

    AABB aabb;

    std::vector<LOD> lods;
    std::unordered_map<std::string, Material> materials;

    bool upload();
    void unload();
};
