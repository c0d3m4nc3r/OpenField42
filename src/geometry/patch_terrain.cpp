#include "geometry/patch_terrain.h"
#include "geometry/template.h"
#include "render/texture.h"
#include "world/water.h"
#include "utils/log.h"
#include "vfs/vfs.h"

#define TILE_SIZE 64
#define VERTS_PER_TILE TILE_SIZE + 1
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
                
                if (VFS::exists(path))
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

bool PatchTerrain::load(const GeometryTemplate* tmpl)
{
    if (tmpl->type != GeometryType::PatchTerrain)
    {
        LOG_ERROR("PatchTerrain::load: Given template type is not PatchTerrain!");
        return false;
    }

    const float scale_xz = static_cast<float>(tmpl->world_size) / (tmpl->material_size - 1);
    const int tiles_per_side = std::max(1, tmpl->material_size / TILE_SIZE);

    // 1. Load heightmap
    
    auto heightmap_data = VFS::readFileData(tmpl->file);
    if (heightmap_data.empty())
    {
        LOG_ERROR("PatchTerrain::load: Failed to read heightmap data from '%s'!", tmpl->file.c_str());
        return false;
    }

    _size = tmpl->material_size;
    _world_size = tmpl->world_size;

    _heightmap.clear();
    _heightmap.reserve(_size * _size);

    const uint16_t* data16 = reinterpret_cast<const uint16_t*>(heightmap_data.data());
    size_t num_elements = heightmap_data.size() / sizeof(uint16_t);

    for (size_t i = 0; i < num_elements; ++i)
    {
        uint16_t hh = data16[i];
        float height_value = (((float)hh / (float)UINT16_MAX) * (float)MAX_HEIGHT) * tmpl->y_scale;
        _heightmap.push_back(height_value);
    }

    // 2. Create materials
    
    materials["base"] = Material();
    auto& base_mat = materials["base"];
    
    // 3. Load textures

    auto texture_paths = genTexturePaths(
        tmpl->tex_base_name,
        tmpl->tex_offset_x,
        tmpl->tex_offset_y,
        tiles_per_side
    );

    base_mat.texture = Texture::loadAtlas(texture_paths, 1024, 1024);
    if (!base_mat.texture)
    {
        LOG_ERROR("PatchTerrain::load: Failed to load textures!");
        return false;
    }

    if (!tmpl->detail_tex_name.empty())
    {
        base_mat.detail_texture = Texture::load(tmpl->detail_tex_name);
        if (!base_mat.detail_texture)
        {
            LOG_WARNING("PatchTerrain::load: Failed to load detail texture!");
        }
    }

    // 4. Generate terrain meshes

    glm::vec3 global_min(FLT_MAX);
    glm::vec3 global_max(-FLT_MAX);

    LOD& lod = lods.emplace_back();
    lod.meshes.reserve(tiles_per_side * tiles_per_side);

    for (int tz = 0; tz < tiles_per_side; ++tz)
    {
        for (int tx = 0; tx < tiles_per_side; ++tx)
        {
            const int start_x = tx * VERTS_PER_TILE;
            const int start_z = tz * VERTS_PER_TILE;
    
            const int end_x = std::min(start_x + VERTS_PER_TILE, _size);
            const int end_z = std::min(start_z + VERTS_PER_TILE, _size);
    
            const int w = (end_x - start_x);
            const int h = (end_z - start_z);
    
            if (w <= 0 || h <= 0) continue;
    
            std::vector<Vertex> vertices;
            vertices.reserve(w * h);
    
            glm::vec3 chunk_min(FLT_MAX);
            glm::vec3 chunk_max(-FLT_MAX);
    
            for (int z = start_z; z < end_z; ++z)
            {
                for (int x = start_x; x < end_x; ++x)
                {
                    Vertex v{};
                    v.position = {
                        (float)x * scale_xz,
                        getHeightAt(x, z),
                        (float)z * scale_xz
                    };
                    v.normal = { 0.0f, 1.0f, 0.0f };
                    v.uv = {
                        (float)x / ((float)_size - 1.0f),
                        (float)z / ((float)_size - 1.0f)
                    };
                    v.color = {1.0f, 1.0f, 1.0f, 1.0f};
        
                    chunk_min = glm::min(chunk_min, v.position);
                    chunk_max = glm::max(chunk_max, v.position);
                    global_min = glm::min(global_min, v.position);
                    global_max = glm::max(global_max, v.position);
        
                    vertices.push_back(v);
                }
            }
    
            std::vector<unsigned int> indices;
            indices.reserve((w - 1) * (h - 1) * 6);
            for (int z = 0; z < h - 1; ++z)
            {
                for (int x = 0; x < w - 1; ++x)
                {
                    unsigned int BL = z * w + x;
                    unsigned int BR = z * w + (x + 1);
                    unsigned int TL = (z + 1) * w + x;
                    unsigned int TR = (z + 1) * w + (x + 1);
        
                    indices.push_back(BL);
                    indices.push_back(TL);
                    indices.push_back(BR);
        
                    indices.push_back(BR);
                    indices.push_back(TL);
                    indices.push_back(TR);
                }
            }
    
            auto& mesh = lod.meshes.emplace_back(std::move(vertices), std::move(indices));
            mesh.material = &base_mat;
            mesh.aabb = AABB(chunk_min, chunk_max);
        }
    }

    g_Water.init((float)_size / 4, (float)tmpl->world_size / ((float)_size / 4 - 1.0f), (float)tmpl->water_level);

    aabb = AABB(global_min, global_max);

    return true;
}

int PatchTerrain::getSize() const { return _size; }
int PatchTerrain::getWorldSize() const { return _world_size; }

float PatchTerrain::getHeightAt(int x, int z) const
{
    if (x < 0 || x >= _size || z < 0 || z >= _size)
        return 0.0f;

    return _heightmap[z * _size + x];
}

float PatchTerrain::getHeightAtWorld(float x, float z) const
{
    int local_x = static_cast<int>((x / _world_size) * (_size - 1));
    int local_z = static_cast<int>((z / _world_size) * (_size - 1));
    return getHeightAt(local_x, local_z);
}
