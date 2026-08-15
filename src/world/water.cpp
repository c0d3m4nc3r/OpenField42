#include "world/water.h"

#include "core/globals.h"
#include "world/terrain.h"
#include "world/world.h"

#include "glad/gl.h"

#include <glm/common.hpp>

bool Water::init()
{
    LOG_INFO("Water::init: Initializing water...");

    const int TILE_SIZE = 64/4;
    const int step = 4;
    const int CHUNK_VERTS_PER_SIDE = (TILE_SIZE / step) + 1;
    const int CHUNK_VERT_COUNT = CHUNK_VERTS_PER_SIDE * CHUNK_VERTS_PER_SIDE;

    Terrain& terrain = g_World->getTerrain();

    float scale_xz = static_cast<float>(terrain.getWorldSize()) / static_cast<float>(terrain.getSize());

    int num_chunks_x = terrain.getSize() / TILE_SIZE;
    int num_chunks_z = terrain.getSize() / TILE_SIZE;

    auto& lod = _geometry.lods.emplace_back();
    
    lod.vertices.reserve(num_chunks_x * num_chunks_z * CHUNK_VERT_COUNT);
    
    unsigned int current_vertex_offset = 0;
    unsigned int current_index_offset = 0;

    glm::vec3 global_min(FLT_MAX);
    glm::vec3 global_max(-FLT_MAX);

    for (int cz = 0; cz < num_chunks_z; cz++)
    {
        for (int cx = 0; cx < num_chunks_x; cx++)
        {
            int terrain_start_x = cx * TILE_SIZE;
            int terrain_start_z = cz * TILE_SIZE;

            bool has_visible_water = false;
            for (int z = 0; z < CHUNK_VERTS_PER_SIDE; z++)
            {
                for (int x = 0; x < CHUNK_VERTS_PER_SIDE; x++)
                {
                    int tx = std::min(terrain_start_x + x * step, terrain.getSize());
                    int tz = std::min(terrain_start_z + z * step, terrain.getSize());
                    
                    if (terrain.getWaterHeight() > terrain.getHeight(tx, tz))
                    {
                        has_visible_water = true;
                        break;
                    }
                }
                if (has_visible_water) break;
            }

            if (!has_visible_water) continue;

            glm::vec3 chunk_min(FLT_MAX);
            glm::vec3 chunk_max(-FLT_MAX);

            for (int z = 0; z < CHUNK_VERTS_PER_SIDE; z++)
            {
                for (int x = 0; x < CHUNK_VERTS_PER_SIDE; x++)
                {
                    Geometry::Vertex v;
                    int tx = std::min(terrain_start_x + x * step, terrain.getSize());
                    int tz = std::min(terrain_start_z + z * step, terrain.getSize());

                    v.position = {
                        static_cast<float>(tx * scale_xz),
                        static_cast<float>(terrain.getWaterHeight()),
                        static_cast<float>(tz * scale_xz)
                    };

                    v.normal = {0.0f, 1.0f, 0.0f};
                    v.uv.x = (v.position.x / static_cast<float>(terrain.getWorldSize())) * 64.0f;
                    v.uv.y = (v.position.z / static_cast<float>(terrain.getWorldSize())) * 64.0f;

                    float depth = terrain.getWaterHeight() - terrain.getHeight(tx, tz);
                    float t = glm::clamp(depth / _color_depth, 0.0f, 1.0f);
                    glm::vec3 color = glm::mix(_color.toVec3(), _deep_color.toVec3(), t);
                    float alpha = glm::mix(_shallow_alpha, 1.0f, t);
                    v.color = glm::vec4(color, alpha);

                    chunk_min = glm::min(chunk_min, v.position);
                    chunk_max = glm::max(chunk_max, v.position);
                    global_min = glm::min(global_min, v.position);
                    global_max = glm::max(global_max, v.position);

                    lod.vertices.push_back(v);
                }
            }

            unsigned int local_indices_count = 0;
            for (int z = 0; z < CHUNK_VERTS_PER_SIDE - 1; z++)
            {
                for (int x = 0; x < CHUNK_VERTS_PER_SIDE - 1; x++)
                {
                    unsigned int v0 = z * CHUNK_VERTS_PER_SIDE + x;
                    unsigned int v1 = (z + 1) * CHUNK_VERTS_PER_SIDE + x;
                    unsigned int v2 = z * CHUNK_VERTS_PER_SIDE + (x + 1);
                    unsigned int v3 = (z + 1) * CHUNK_VERTS_PER_SIDE + (x + 1);
            
                    lod.indices.push_back(v0);
                    lod.indices.push_back(v1);
                    lod.indices.push_back(v2);
            
                    lod.indices.push_back(v2);
                    lod.indices.push_back(v1);
                    lod.indices.push_back(v3);

                    local_indices_count += 6;
                }
            }

            auto& mesh = lod.meshes.emplace_back();
            mesh.index_count = local_indices_count;
            mesh.index_start = current_index_offset;
            mesh.base_vertex = current_vertex_offset;

            mesh.aabb = AABB(chunk_min, chunk_max);

            current_vertex_offset += CHUNK_VERT_COUNT;
            current_index_offset += local_indices_count;
        }
    }

    _geometry.upload();

    _geometry.aabb = AABB(global_min, global_max);
    _geometry.type = GeometryType::WaterMesh;

    LOG_INFO("Water::init: Water initialized!");

    return true;
}

void Water::shutdown()
{
    for (auto& layer : _layers)
    {
        layer.texture = { INVALID_TEXTURE_ID };
        layer.scroll_dir = glm::vec2(0.0f);
        layer.scroll_speed = 0.0f;
        layer.uv_scale = 1.0f;
    }

    _geometry.unload();

    LOG_INFO("Water::shutdown: Water shutdown");
}
