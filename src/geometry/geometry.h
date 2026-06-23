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
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        Material* material = nullptr;
        AABB aabb;
        int index_count = 0;
        int poly_count = 0;
        bool use_geom_aabb = false;
        bool uploaded = false;

        unsigned int vao, vbo, ebo;
 
        uint32_t source_stride = sizeof(Vertex);

        Mesh(
            const std::vector<Vertex>& vertices = {},
            const std::vector<unsigned int>& indices = {}
        );

        ~Mesh();

        bool upload();
        void unload();
        
        void draw(Shader* shader) const;
    };

    struct LOD
    {
        std::vector<Mesh> meshes;
    };

    virtual ~Geometry() = default;

    GeometryType type = GeometryType::Unknown;

    AABB aabb;

    std::vector<LOD> lods;
    std::unordered_map<std::string, Material> materials;

    void draw(Shader* shader, const glm::mat4& model) const;

    bool upload();
 
};
