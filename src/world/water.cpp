#include "world/water.h"
#include "world/terrain.h"

#include "glad/gl.h"

#include <glm/common.hpp>

bool Water::init(const Terrain& terrain)
{
    LOG_INFO("Water::init: Initializing water...");

    int terrain_size = terrain.getSize();
    float world_size = static_cast<float>(terrain.getWorldSize());
    int step = 4;
    
    float scale_xz = world_size / static_cast<float>(terrain_size);
    int size = (terrain_size + step - 1) / step + 1;

    auto& lod = _geometry.lods.emplace_back();
    auto& mesh = lod.meshes.emplace_back();
    mesh.index_count = (size - 1) * (size - 1) * 6;
    mesh.index_start = 0;
    mesh.base_vertex = 0;
    
    lod.vertices.reserve(size * size);

    for (int z = 0; z < size; z++)
    {
        for (int x = 0; x < size; x++)
        {
            Geometry::Vertex v;

            int tx = x * step;
            int tz = z * step;
            
            if (tx > terrain_size) tx = terrain_size;
            if (tz > terrain_size) tz = terrain_size;

            v.position = {
                static_cast<float>(tx * scale_xz),
                static_cast<float>(terrain.getWaterHeight()),
                static_cast<float>(tz * scale_xz)
            };

            v.normal = {0.0f, 1.0f, 0.0f};

            v.uv.x = (v.position.x / world_size) * 64.0f;
            v.uv.y = (v.position.z / world_size) * 64.0f;

            float depth = terrain.getWaterHeight() - terrain.getHeight(tx, tz);

            float t = glm::clamp(depth / _color_depth, 0.0f, 1.0f);
            
            glm::vec3 color = glm::mix(_color.toVec3(), _deep_color.toVec3(), t);
            float alpha = glm::mix(_shallow_alpha, 1.0f, t);
            
            v.color = glm::vec4(color, alpha);

            lod.vertices.push_back(v);
        }
    }

    lod.indices.reserve((size - 1) * (size - 1) * 6);

    for (int z = 0; z < size - 1; z++)
    {
        for (int x = 0; x < size - 1; x++)
        {
            unsigned int v0 = z * size + x;
            unsigned int v1 = (z + 1) * size + x;
            unsigned int v2 = z * size + (x + 1);
            unsigned int v3 = (z + 1) * size + (x + 1);
    
            lod.indices.push_back(v0);
            lod.indices.push_back(v1);
            lod.indices.push_back(v2);
    
            lod.indices.push_back(v2);
            lod.indices.push_back(v1);
            lod.indices.push_back(v3);
        }
    }

    _geometry.upload();
    
    _geometry.type = GeometryType::WaterMesh;

    LOG_INFO("Water::init: Water initialized!");

    return true;
}

void Water::shutdown()
{
    for (auto& layer : _layers)
    {
        layer.texture.reset();
        layer.scroll_dir = glm::vec2(0.0f);
        layer.scroll_speed = 0.0f;
        layer.uv_scale = 1.0f;
    }

    _geometry.unload();

    LOG_INFO("Water::shutdown: Water shutdown");
}
