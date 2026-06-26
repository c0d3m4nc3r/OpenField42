#include "geometry/standard_mesh.h"
#include "geometry/material.h"
#include "geometry/geometry_template.h"
#include "utils/log.h"
#include "vfs/vfs.h"
#include "utils/string_utils.h"

#include <cstring>

#define READ_DATA(dest, type) \
    do { \
        if (size - (ptr - data.data()) < sizeof(type)) { \
            LOG_ERROR("StandardMesh::load: Not enough data in '%s'!", full_path.c_str()); \
            return false; \
        } \
        std::memcpy(&dest, ptr, sizeof(type)); \
        ptr += sizeof(type); \
    } while (0)

bool StandardMesh::load(const GeometryTemplate* tmpl)
{
    if (tmpl->type != GeometryType::StandardMesh)
    {
        LOG_ERROR("StandardMesh::load: Given template type is not StandardMesh!");
        return false;
    }

    if (tmpl->file.empty())
    {
        LOG_ERROR("StandardMesh::load: Path is empty!");
        return false;
    }

    std::string full_path = "standardMesh/" + tmpl->file + ".sm";

    auto data = VFS::readFileData(full_path);
    if (data.empty())
    {
        LOG_ERROR("StandardMesh::load: Failed to read data from '%s'!", full_path.c_str());
        return false;
    }

    const char* ptr = data.data();
    size_t size = data.size();

    // 1. Materials

    full_path = "standardMesh/" + tmpl->file + ".rs";

    auto mats = Material::load(full_path);
    if (mats.empty())
    {
        LOG_ERROR("StandardMesh::load: Failed to load materials from '%s'!", full_path.c_str());
        return false;
    }

    for (auto& material : mats)
    {
        materials[material.name] = std::move(material);
    }

    // 2. Header

    uint32_t version;
    READ_DATA(version, uint32_t);

    ptr += sizeof(uint32_t); // skip unknown

    READ_DATA(aabb.min, glm::vec3);
    READ_DATA(aabb.max, glm::vec3);

    if (version > 9)
    {
        ptr += sizeof(uint8_t); // skip qflag
    }

    // 3. Collision meshes

    uint32_t col_mesh_num;
    READ_DATA(col_mesh_num, uint32_t);

    // skip collision meshes
    for (uint32_t i = 0; i < col_mesh_num; i++)
    {
        uint32_t block_size;
        READ_DATA(block_size, uint32_t);
        ptr += block_size;
    }

    // 4. LODs

    uint32_t lod_num;
    READ_DATA(lod_num, uint32_t);
    lods.resize(lod_num);

    for (uint32_t i = 0; i < lod_num; i++)
    {
        auto& lod = lods[i];

        uint32_t mesh_num;
        READ_DATA(mesh_num, uint32_t);
        lod.meshes.resize(mesh_num);

        struct MeshInfo
        {
            uint32_t vertex_count;
            uint32_t index_count;
            uint32_t vertex_stride;
        };

        std::vector<MeshInfo> mesh_infos;
        mesh_infos.reserve(mesh_num);

        uint32_t total_vertices = 0;
        uint32_t total_indices = 0;

        for (uint32_t j = 0; j < mesh_num; j++)
        {
            Mesh& mesh = lods[i].meshes[j];

            uint32_t mat_name_len;
            READ_DATA(mat_name_len, uint32_t);

            std::string mat_name;
            mat_name.assign(ptr, mat_name_len);
            ptr += mat_name_len;

            auto it = materials.find(StringUtils::lowercase(mat_name));
            if (it != materials.end()) {
                mesh.material = &it->second;
            } else {
                LOG_WARNING("StandardMesh::load: Material '%s' not found for '%s'!", mat_name.c_str(), tmpl->name.c_str());
                mesh.material = nullptr;
            }

            ptr += sizeof(uint32_t) * 3; // skip unknown

            uint32_t prim_type;
            READ_DATA(prim_type, uint32_t);
            assert(prim_type == 4);

            ptr += sizeof(uint32_t); // skip unknown

            auto& info = mesh_infos.emplace_back();

            READ_DATA(info.vertex_stride, uint32_t);
            READ_DATA(info.vertex_count, uint32_t);
            READ_DATA(info.index_count, uint32_t);

            total_vertices += info.vertex_count;
            total_indices += info.index_count;

            ptr += sizeof(uint32_t); // skip unknown
        }

        lod.vertices.reserve(total_vertices);
        lod.indices.reserve(total_indices);

        for (uint32_t j = 0; j < mesh_num; j++)
        {
            auto& mesh = lod.meshes[j];
            auto& info = mesh_infos[j];

            mesh.index_count = info.index_count;
            mesh.index_start = (uint32_t)lod.indices.size();
            mesh.base_vertex = (uint32_t)lod.vertices.size();

            for (uint32_t v = 0; v < info.vertex_count; v++)
            {
                Vertex vertex;
    
                READ_DATA(vertex.position, glm::vec3);
                READ_DATA(vertex.normal, glm::vec3);
                READ_DATA(vertex.uv, glm::vec2);
                vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};

                lod.vertices.push_back(vertex);
                
                size_t extra = info.vertex_stride - (sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2));
                ptr += extra;
            }

            for (uint32_t idx = 0; idx < info.index_count; idx++)
            {
                uint16_t raw;
                READ_DATA(raw, uint16_t);
                lod.indices.push_back(raw);
            }
        }
    }
    
    return true;
}
