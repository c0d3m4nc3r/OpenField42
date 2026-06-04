#pragma once

#include "geometry/material.h"
#include "math/aabb.h"

#include <vector>
#include <unordered_map>
#include <string>

enum class GeometryType : unsigned char
{
    Unknown,
    AnimatedMesh,
    SkeletonCollisionMesh,
    StandardMesh,
    TreeMesh,
    PatchTerrain
};

std::string geometryTypeToString(GeometryType type);
GeometryType geometryTypeFromString(const std::string& str);

struct GeometryTemplate;

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
        
        void draw(Shader* shader);
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

    static inline std::unordered_map<std::string, std::unique_ptr<Geometry>> cache;
    static Geometry* create(const GeometryTemplate* tmpl);
    static bool uploadAll();

    virtual void draw(Shader* shader, const glm::mat4& model);

    bool upload();
 
};
