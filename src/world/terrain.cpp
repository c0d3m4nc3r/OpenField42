#include "world/terrain.h"

#include "core/globals.h"
#include "geometry/geometry.h"
#include "geometry/geometry_template.h"
#include "vfs/vfs.h"

#define TILE_SIZE 64
#define VERTS_PER_TILE (TILE_SIZE + 1)
#define MAX_HEIGHT 256
#define DEBUG_TEX "textures/debug.png"

static std::vector<std::string> genTexturePaths(const std::string& base_path, int offset_x, int offset_y, int tiles_per_side)
{
    std::vector<std::string> texture_paths;
    texture_paths.reserve(tiles_per_side * tiles_per_side);

    for (int y = 0; y < tiles_per_side; y++)
    {
        for (int x = 0; x < tiles_per_side; x++)
        {
            if (x >= offset_x &&
                y >= offset_y && 
                x < offset_x + tiles_per_side &&
                y < offset_y + tiles_per_side)
            {
                int src_x = x - offset_x;
                int src_y = y - offset_y;
                
                std::string path = base_path +
                                (src_x < 10 ? "0" : "") + std::to_string(src_x) + "x" +
                                (src_y < 10 ? "0" : "") + std::to_string(src_y) + ".dds";
                
                if (g_VFS->exists(path))
                    texture_paths.push_back(path);
                else
                    texture_paths.push_back(DEBUG_TEX);
            }
            else
            {
                texture_paths.push_back(DEBUG_TEX);
            }
        }
    }

    return texture_paths;
}

bool Terrain::init(const GeometryTemplate* tmpl)
{
    LOG_INFO("Terrain::init: Initializing terrain...");

    const float scale_xz = static_cast<float>(tmpl->world_size) / tmpl->material_size;
    LOG_DEBUG("Terrain::init: Info: Size: %dx%d, Scale XZ: %.2f, Scale Y: %.2f",
        tmpl->material_size, tmpl->material_size, scale_xz, tmpl->y_scale);
    
    const int tiles_per_side = std::max(1, tmpl->material_size / TILE_SIZE);
    
    // 1. Load heightmap
    
    auto heightmap_data = g_VFS->readFileData(tmpl->file);
    if (heightmap_data.empty())
    {
        LOG_ERROR("Terrain::init: Failed to read heightmap data from '%s'!", tmpl->file.c_str());
        return false;
    }

    setSize(tmpl->material_size);
    setWorldSize(tmpl->world_size);
    setWaterHeight(tmpl->water_level);

    const uint16_t* data16 = reinterpret_cast<const uint16_t*>(heightmap_data.data());
    size_t num_elements = heightmap_data.size() / sizeof(uint16_t);

    for (size_t i = 0; i < num_elements; ++i)
    {
        uint16_t hh = data16[i];
        float height_value = (((float)hh / (float)UINT16_MAX) * (float)MAX_HEIGHT) * tmpl->y_scale;
        
        int z = static_cast<int>(i / tmpl->material_size);
        int x = static_cast<int>(i % tmpl->material_size);

        setHeight(x, z, height_value);
    }

    // 2. Create geometry

    _geometry = std::make_unique<Geometry>();
    _geometry->type = GeometryType::PatchTerrain;
    
    // 3. Load textures

    auto texture_paths = genTexturePaths(
        tmpl->tex_base_name,
        tmpl->tex_offset_x,
        tmpl->tex_offset_y,
        tiles_per_side
    );

    _base_tex = g_TextureMgr->loadAtlas(texture_paths, 1024, 1024, 3);
    if (!_base_tex.isValid())
    {
        LOG_WARNING("Terrain::init: Failed to load textures!");
    }

    if (!tmpl->detail_tex_name.empty())
    {
        _detail_tex = g_TextureMgr->load(tmpl->detail_tex_name);
        if (!_detail_tex.isValid())
        {
            LOG_WARNING("Terrain::init: Failed to load detail texture!");
        }
    }

    // 4. Generate terrain meshes

    glm::vec3 global_min(FLT_MAX);
    glm::vec3 global_max(-FLT_MAX);

    Geometry::LOD& lod = _geometry->lods.emplace_back();
    lod.meshes.reserve(tiles_per_side * tiles_per_side);

    for (int tz = 0; tz < tiles_per_side; ++tz)
    {
        for (int tx = 0; tx < tiles_per_side; ++tx)
        {
            const int start_x = tx * TILE_SIZE;
            const int start_z = tz * TILE_SIZE;
    
            const int end_x = std::min(start_x + VERTS_PER_TILE, tmpl->material_size + 1);
            const int end_z = std::min(start_z + VERTS_PER_TILE, tmpl->material_size + 1);
    
            const int w = (end_x - start_x);
            const int h = (end_z - start_z);
    
            if (w <= 0 || h <= 0) continue;

            auto& mesh = lod.meshes.emplace_back();
            mesh.index_count = static_cast<uint32_t>((w - 1) * (h - 1) * 6);
            mesh.index_start = static_cast<uint32_t>(lod.indices.size());
            mesh.base_vertex = static_cast<uint32_t>(lod.vertices.size());

            glm::vec3 chunk_min(FLT_MAX);
            glm::vec3 chunk_max(-FLT_MAX);

            lod.vertices.reserve(w * h);
    
            for (int z = start_z; z < end_z; ++z)
            {
                for (int x = start_x; x < end_x; ++x)
                {
                    Geometry::Vertex v{};
                    v.position = {
                        (float)x * scale_xz,
                        getHeight(x, z),
                        (float)z * scale_xz
                    };
                    v.normal = { 0.0f, 1.0f, 0.0f };
                    v.uv = {
                        (float)x / (float)tmpl->material_size,
                        (float)z / (float)tmpl->material_size
                    };
                    v.color = {1.0f, 1.0f, 1.0f, 1.0f};
                    
                    global_min = glm::min(global_min, v.position);
                    global_max = glm::max(global_max, v.position);
                    chunk_min = glm::min(chunk_min, v.position);
                    chunk_max = glm::max(chunk_max, v.position);

                    lod.vertices.push_back(v);
                }
            }
    
            lod.indices.reserve((w - 1) * (h - 1) * 6);
            for (int z = 0; z < h - 1; ++z)
            {
                for (int x = 0; x < w - 1; ++x)
                {
                    unsigned int BL = z * w + x;
                    unsigned int BR = z * w + (x + 1);
                    unsigned int TL = (z + 1) * w + x;
                    unsigned int TR = (z + 1) * w + (x + 1);
        
                    lod.indices.push_back(BL);
                    lod.indices.push_back(TL);
                    lod.indices.push_back(BR);
        
                    lod.indices.push_back(BR);
                    lod.indices.push_back(TL);
                    lod.indices.push_back(TR);
                }
            }

            mesh.aabb = AABB(chunk_min, chunk_max);
        }
    }

    _geometry->aabb = AABB(global_min, global_max);

    _geometry->upload();

    LOG_INFO("Terrain::init: Terrain initialized!");

    return true;
}

void Terrain::shutdown()
{
    _base_tex = { INVALID_TEXTURE_ID };
    _detail_tex = { INVALID_TEXTURE_ID };
    _geometry.reset();
    _heights.clear();

    _size = 0;
    _world_size = 0;
    _water_height = 0;

    LOG_INFO("Terrain::shutdown: Terrain shutdown!");
}
