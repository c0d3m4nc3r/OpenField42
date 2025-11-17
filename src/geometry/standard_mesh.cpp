#include "geometry/standard_mesh.h"
#include "geometry/material.h"
#include "geometry/template.h"
#include "core/log.h"
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
        uint32_t mesh_num;
        READ_DATA(mesh_num, uint32_t);
        lods[i].meshes.resize(mesh_num);

        for (uint32_t j = 0; j < mesh_num; j++)
        {
            Mesh& mesh = lods[i].meshes[j];
            mesh.use_geom_aabb = true;

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

            uint32_t vert_stride, vert_num, index_num;
            READ_DATA(vert_stride, uint32_t);
            READ_DATA(vert_num, uint32_t);
            READ_DATA(index_num, uint32_t);

            ptr += sizeof(uint32_t); // skip unknown

            mesh.vertices.resize(vert_num);
            mesh.indices.resize(index_num);

            mesh.source_stride = vert_stride;
        }

        for (uint32_t j = 0; j < mesh_num; j++)
        {
            auto& mesh = lods[i].meshes[j];

            for (uint32_t v = 0; v < mesh.vertices.size(); v++)
            {
                auto& vertex = mesh.vertices[v];
    
                READ_DATA(vertex.position, glm::vec3);
                READ_DATA(vertex.normal, glm::vec3);
                READ_DATA(vertex.uv, glm::vec2);
                
                size_t extra = mesh.source_stride - (sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2));
                ptr += extra;
            }

            for (uint32_t idx = 0; idx < mesh.indices.size(); idx++)
            {
                int16_t raw;
                READ_DATA(raw, int16_t);
                mesh.indices[idx] = static_cast<unsigned int>(raw);
            }
        }
    }

    return true;
}
