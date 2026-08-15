#include "geometry/tree_mesh.h"

#include "core/globals.h"
#include "geometry/geometry_template.h"
#include "render/texture.h"
#include "utils/log.h"
#include "vfs/vfs.h"

#include <cstring>

#define READ_DATA(dest, bytes_count) \
    do { \
        size_t num_bytes = (bytes_count); \
        if (size - (ptr - data.data()) < num_bytes) { \
            LOG_ERROR("TreeMesh::load: Not enough data in '%s'!", full_path.c_str()); \
            return false; \
        } \
        std::memcpy(&dest, ptr, num_bytes); \
        ptr += num_bytes; \
    } while (0)

#define READ_STRING(dest) \
    do { \
        uint32_t str_len = 0; \
        READ_DATA(str_len, sizeof(uint32_t)); \
        \
        if (size - (ptr - data.data()) < str_len) { \
            LOG_ERROR("TreeMesh::load: Not enough data for string in '%s'!", full_path.c_str()); \
            return false; \
        } \
        \
        (dest).resize(str_len); \
        if (str_len > 0) { \
            std::memcpy(&(dest)[0], ptr, str_len); \
            ptr += str_len; \
        } \
    } while (0)

constexpr int32_t COLLISION_MAGIC = -342375686; // 0xEBB9E0BA

struct TM_Block
{
    uint32_t index_start;
    uint32_t primitive_count;
    std::string texture_name;
};

bool TreeMesh::load(const GeometryTemplate* tmpl)
{
    assert(tmpl != nullptr);

    if (tmpl->type != GeometryType::TreeMesh)
    {
        LOG_ERROR("TreeMesh::load: Given template type is not TreeMesh!");
        return false;
    }

    if (tmpl->file.empty())
    {
        LOG_ERROR("TreeMesh::load: Path is empty!");
        return false;
    }

    LOG_INFO("TreeMesh::load: Loading TreeMesh from '%s'...", tmpl->file.c_str());

    std::string full_path = "treeMesh/" + tmpl->file + ".tm";

    std::vector<char> data = VFS::readFileData(full_path);
    if (data.empty())
    {
        LOG_ERROR("TreeMesh::load: Failed to read data from '%s'!", full_path.c_str());
        return false;
    }

    const char* ptr = data.data();
    size_t size = data.size();

    // Header

    uint32_t version, sub_version;
    uint32_t angle_count;

    READ_DATA(version, sizeof(uint32_t));

    if (version != 3)
    {
        LOG_ERROR("TreeMesh::load: Unsupported version: %u", version);
        return false;
    }

    READ_DATA(sub_version, sizeof(uint32_t));
    READ_DATA(angle_count, sizeof(uint32_t));

    LOG_DEBUG("TreeMesh::load: Version: %u, Subversion: %u", version, sub_version);
    LOG_DEBUG("TreeMesh::load: Angle count: %u", angle_count);

    // Mesh AABB

    READ_DATA(aabb.min, sizeof(glm::vec3));
    READ_DATA(aabb.max, sizeof(glm::vec3));

    LOG_DEBUG("TreeMesh::load: Mesh AABB: Min: (%.2f, %.2f, %.2f),  Max: (%.2f, %.2f, %.2f)",
        aabb.min.x, aabb.min.y, aabb.min.z,
        aabb.max.x, aabb.max.y, aabb.max.z);

    // Sprites AABB

    AABB sprites_aabb;

    READ_DATA(sprites_aabb.min, sizeof(glm::vec3));
    READ_DATA(sprites_aabb.max, sizeof(glm::vec3));

    LOG_DEBUG("TreeMesh::load: Sprites AABB: Min: (%.2f, %.2f, %.2f),  Max: (%.2f, %.2f, %.2f)",
        sprites_aabb.min.x, sprites_aabb.min.y, sprites_aabb.min.z,
        sprites_aabb.max.x, sprites_aabb.max.y, sprites_aabb.max.z);

    // Blocks

    auto readTMBlocks = [&](std::vector<TM_Block>& dest) -> bool{
        uint32_t count;
        READ_DATA(count, sizeof(uint32_t));

        for (uint32_t i = 0; i < count; ++i)
        {
            auto& block = dest.emplace_back();
            READ_DATA(block.index_start, sizeof(uint32_t));
            READ_DATA(block.primitive_count, sizeof(uint32_t));
            READ_STRING(block.texture_name);
        }

        return true;
    };

    std::vector<TM_Block> branches, trunks, sprites, billboards;

    struct BlockTarget
    {
        std::vector<TM_Block>& vec;
        const char* name;
    };

    BlockTarget targets[] = {
        { branches,   "Branches" },
        { trunks,     "Trunks" },
        { sprites,    "Sprites" },
        { billboards, "Billboards" }
    };

    for (auto& target : targets)
    {
        if (!readTMBlocks(target.vec))
        {
            LOG_ERROR("TreeMesh::load: Failed to load %s!", target.name);
            return false;
        }
        LOG_DEBUG("TreeMesh::load: %s: %zu", target.name, target.vec.size());
        
        for (size_t i = 0; i < target.vec.size(); ++i)
        {
            auto& block = target.vec[i];
            LOG_DEBUG("\tBlock %zu: Index Start: %u, Primitive Count: %u, Texture Name: %s",
                i, block.index_start, block.primitive_count, block.texture_name.c_str());
        }
    }

    // Collision Mesh

    int32_t col_magic;
    READ_DATA(col_magic, sizeof(int32_t));

    if (col_magic == COLLISION_MAGIC)
    {
        LOG_DEBUG("TreeMesh::load: Mesh has collision :(");
        uint32_t col_version;
        READ_DATA(col_version, sizeof(uint32_t));

        if (col_version == 5)
        {
            uint32_t num_verts;
            READ_DATA(num_verts, sizeof(uint32_t));

            // 3 floats (pos) + 1 float (padding) = 16 bytes
            ptr += num_verts * 16; // skip collision vertices

            uint32_t num_faces;
            READ_DATA(num_faces, sizeof(uint32_t));

            // 3 × int16 (indices) + 1 × int16 (matID) = 8 bytes
            ptr += num_faces * 8; // skip collision faces

            LOG_DEBUG("TreeMesh::load: Collision mesh vertices: %u", num_verts);
            LOG_DEBUG("TreeMesh::load: Collision mesh faces: %u", num_faces);

            auto bypassBSPNode = [&](auto&& self) -> bool {
                ptr += 24; // plane (4 floats * 4 bytes + 8 bytes)
                
                uint32_t idxCnt;
                READ_DATA(idxCnt, sizeof(uint32_t));

                ptr += idxCnt * 4;
                
                uint8_t tmp;

                READ_DATA(tmp, sizeof(uint8_t));
                if (tmp == 1) self(self); // above node
                
                READ_DATA(tmp, sizeof(uint8_t));
                if (tmp == 1) self(self); // below node

                return true;
            };

            // uint32 (total_face_list_cnt) + uint32 (total_bsp_node_cnt) = 8 bytes
            ptr += 8;
            
            uint32_t total_face_cnt;
            READ_DATA(total_face_cnt, sizeof(uint32_t));
            ptr += total_face_cnt * 32;
            bypassBSPNode(bypassBSPNode);

        } else {
            LOG_WARNING("TreeMesh::load: Unsupported collision version: %u", col_version);
        }
    } else {
        LOG_DEBUG("TreeMesh::load: Mesh has no collision :)");
    }

    // Visible Mesh

    auto& lod = lods.emplace_back();

    uint32_t num_verts;
    READ_DATA(num_verts, sizeof(uint32_t));
    lod.vertices.resize(num_verts);

    LOG_DEBUG("TreeMesh::load: Visible mesh vertices: %u", num_verts);
    
    for (uint32_t i = 0; i < num_verts; ++i)
    {
        auto& v = lod.vertices[i];
        READ_DATA(v.position, sizeof(glm::vec3));
        READ_DATA(v.normal, sizeof(glm::vec3));

        ptr += sizeof(uint32_t); // color?
        
        READ_DATA(v.uv, sizeof(glm::vec2));
        READ_DATA(v.sprite_ofs, sizeof(glm::vec2));
    }

    uint32_t num_indices;
    READ_DATA(num_indices, sizeof(uint32_t));

    LOG_DEBUG("TreeMesh::load: Visible mesh indices: %u", num_indices);

    lod.indices.resize(num_indices);
    
    for (uint32_t i = 0; i < num_indices; ++i)
    {
        READ_DATA(lod.indices[i], sizeof(uint16_t));
    }

    auto getMat = [this](const std::string& texture_name, bool is_billboard) -> Material&
    {
        std::string mat_key = texture_name;
        if (is_billboard)   mat_key += "_bb";

        auto [it, inserted] = materials.try_emplace(mat_key);
        if (inserted)
        {
            it->second.name = texture_name;
            it->second.texture = g_TextureMgr->load(texture_name);
            it->second.billboard = is_billboard;
        }
        return it->second;
    };

    for (const auto& trunk : trunks)
    {
        auto& mesh = lod.meshes.emplace_back();
        mesh.index_count = trunk.primitive_count * 3;
        mesh.index_start = trunk.index_start;
        mesh.base_vertex = 0;
        mesh.material = &getMat(trunk.texture_name, false);
        mesh.material->lighting = true;
    }

    for (const auto& branch : branches)
    {
        auto& mesh = lod.meshes.emplace_back();
        mesh.index_count = branch.primitive_count * 3;
        mesh.index_start = branch.index_start;
        mesh.base_vertex = 0;
        mesh.material = &getMat(branch.texture_name, false);
    }

    for (const auto& sprite : sprites)
    { 
        auto& mesh = lod.meshes.emplace_back();
        mesh.index_count = sprite.primitive_count * 3;
        mesh.index_start = sprite.index_start;
        mesh.base_vertex = 0;
        mesh.material = &getMat(sprite.texture_name, true);
    }

    // TODO: Load billboards

    LOG_INFO("TreeMesh::load: Loaded successfully!");

    return true;
}
