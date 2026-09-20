#include "world/water.h"

#include "core/globals.h"
#include "render/renderer.h"
#include "world/terrain.h"
#include "world/world.h"

#include "glad/gl.h"

#include <glm/common.hpp>

constexpr int CELLS_PER_CHUNK = 16;
constexpr int VERTS_PER_SIDE = CELLS_PER_CHUNK + 1;
constexpr int CHUNK_VERT_COUNT = VERTS_PER_SIDE * VERTS_PER_SIDE;
constexpr int CHUNK_INDEX_COUNT = CELLS_PER_CHUNK * CELLS_PER_CHUNK * 6;

void Water::clear()
{
    for (auto& layer : _layers)
    {
        layer.texture = { INVALID_TEXTURE_ID };
        layer.scroll_dir = glm::vec2(0.0f);
        layer.scroll_speed = 0.0f;
        layer.uv_scale = 1.0f;
    }

    _geometry.unload();

    LOG_INFO("Water::clear: Water cleared!");
}

void Water::generateGeometry()
{
    if (!_geometry.lods.empty())
    {
        _geometry.unload();
        _geometry.lods.clear();
    }
    
    Terrain& terrain = g_World->getTerrain();

    constexpr int CHUNK_SIZE_IN_TERRAIN_SAMPLES = 32; 
    
    float scale_xz = static_cast<float>(terrain.getWorldSize()) / static_cast<float>(terrain.getSize());
    float chunk_world_size = CHUNK_SIZE_IN_TERRAIN_SAMPLES * scale_xz;

    int num_chunks_x = terrain.getSize() / CHUNK_SIZE_IN_TERRAIN_SAMPLES;
    int num_chunks_z = terrain.getSize() / CHUNK_SIZE_IN_TERRAIN_SAMPLES;

    auto& lod = _geometry.lods.emplace_back();
    
    unsigned int current_vertex_offset = 0;
    unsigned int current_index_offset = 0;

    glm::vec3 global_min(FLT_MAX);
    glm::vec3 global_max(-FLT_MAX);

    for (int cz = 0; cz < num_chunks_z; cz++)
    {
        for (int cx = 0; cx < num_chunks_x; cx++)
        {
            int terrain_start_x = cx * CHUNK_SIZE_IN_TERRAIN_SAMPLES;
            int terrain_start_z = cz * CHUNK_SIZE_IN_TERRAIN_SAMPLES;

            bool has_visible_water = false;
            for (int z = 0; z <= CHUNK_SIZE_IN_TERRAIN_SAMPLES; z += 2)
            {
                for (int x = 0; x <= CHUNK_SIZE_IN_TERRAIN_SAMPLES; x += 2)
                {
                    int tx = std::min(terrain_start_x + x, terrain.getSize());
                    int tz = std::min(terrain_start_z + z, terrain.getSize());
                    
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

            for (int z = 0; z < VERTS_PER_SIDE; z++)
            {
                for (int x = 0; x < VERTS_PER_SIDE; x++)
                {
                    Geometry::Vertex v;

                    float step_ratio = static_cast<float>(x) / static_cast<float>(CELLS_PER_CHUNK);
                    float step_ratio_z = static_cast<float>(z) / static_cast<float>(CELLS_PER_CHUNK);

                    int tx = std::min(terrain_start_x + static_cast<int>(step_ratio * CHUNK_SIZE_IN_TERRAIN_SAMPLES), terrain.getSize());
                    int tz = std::min(terrain_start_z + static_cast<int>(step_ratio_z * CHUNK_SIZE_IN_TERRAIN_SAMPLES), terrain.getSize());

                    v.position = {
                        (terrain_start_x + step_ratio * CHUNK_SIZE_IN_TERRAIN_SAMPLES) * scale_xz,
                        static_cast<float>(terrain.getWaterHeight()),
                        (terrain_start_z + step_ratio_z * CHUNK_SIZE_IN_TERRAIN_SAMPLES) * scale_xz
                    };

                    v.normal = {0.0f, 1.0f, 0.0f};

                    v.uvs[0].x = v.position.x / 16.0f;
                    v.uvs[0].y = v.position.z / 16.0f;

                    float depth = terrain.getWaterHeight() - terrain.getHeight(tx, tz);
                    float t = glm::clamp(depth / _color_depth, 0.0f, 1.0f);
                    glm::vec3 color = glm::mix(_color, _deep_color, t);
                    float alpha = glm::mix(_shallow_alpha, 1.0f, t);
                    v.color = glm::vec4(color, alpha);

                    chunk_min = glm::min(chunk_min, v.position);
                    chunk_max = glm::max(chunk_max, v.position);
                    global_min = glm::min(global_min, v.position);
                    global_max = glm::max(global_max, v.position);

                    lod.vertices.push_back(v);
                }
            }

            for (int z = 0; z < CELLS_PER_CHUNK; z++)
            {
                for (int x = 0; x < CELLS_PER_CHUNK; x++)
                {
                    unsigned int v0 = z * VERTS_PER_SIDE + x;
                    unsigned int v1 = (z + 1) * VERTS_PER_SIDE + x;
                    unsigned int v2 = z * VERTS_PER_SIDE + (x + 1);
                    unsigned int v3 = (z + 1) * VERTS_PER_SIDE + (x + 1);
            
                    lod.indices.push_back(v0);
                    lod.indices.push_back(v1);
                    lod.indices.push_back(v2);
            
                    lod.indices.push_back(v2);
                    lod.indices.push_back(v1);
                    lod.indices.push_back(v3);
                }
            }

            auto& mesh = lod.meshes.emplace_back();
            mesh.index_count = CHUNK_INDEX_COUNT;
            mesh.index_start = current_index_offset;
            mesh.base_vertex = current_vertex_offset;

            mesh.aabb = AABB(chunk_min, chunk_max);

            current_vertex_offset += CHUNK_VERT_COUNT;
            current_index_offset += CHUNK_INDEX_COUNT;
        }
    }

    _geometry.upload();
    _geometry.aabb = AABB(global_min, global_max);
    _geometry.type = GeometryType::WaterMesh;
}

void Water::generateHalfVecLUT()
{
    glm::vec3 light_dir = g_Renderer->getSunDirection();
    light_dir.y *= _specular_streak_factor;
    light_dir = glm::normalize(light_dir);

    if (_halfvec_lut != 0)
        glDeleteTextures(1, &_halfvec_lut);

    const int SIZE = 64;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &_halfvec_lut);
    glTextureStorage2D(_halfvec_lut, 1, GL_RGBA8, SIZE, SIZE);

    glTextureParameteri(_halfvec_lut, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(_halfvec_lut, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(_halfvec_lut, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_halfvec_lut, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(_halfvec_lut, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    std::vector<unsigned char> pixels(SIZE * SIZE * 4);

    for (int face = 0; face < 6; face++)
    {
        for (int y = 0; y < SIZE; y++)
        {
            float v = ((float)y + 0.5f) / SIZE;
            float sy = -(v * 2.0f - 1.0f);

            for (int x = 0; x < SIZE; x++)
            {
                float u = ((float)x + 0.5f) / SIZE;
                float sx = u * 2.0f - 1.0f;

                glm::vec3 R;
                switch (face)
                {
                    case 0: R = glm::vec3( 1.0f,   sy,  -sx); break; // +X
                    case 1: R = glm::vec3(-1.0f,   sy,   sx); break; // -X
                    case 2: R = glm::vec3(  sx,  1.0f,  -sy); break; // +Y
                    case 3: R = glm::vec3(  sx, -1.0f,   sy); break; // -Y
                    case 4: R = glm::vec3(  sx,   sy,  1.0f); break; // +Z
                    case 5: R = glm::vec3( -sx,   sy, -1.0f); break; // -Z
                }

                R = glm::normalize(R);
                glm::vec3 result = glm::normalize(R + light_dir);
                glm::ivec3 color = glm::clamp(glm::ivec3((result * 0.5f + 0.5f) * 255.0f), 0, 255);

                int idx = (y * SIZE + x) * 4;
                pixels[idx + 0] = static_cast<unsigned char>(color.r);
                pixels[idx + 1] = static_cast<unsigned char>(color.g);
                pixels[idx + 2] = static_cast<unsigned char>(color.b);
                pixels[idx + 3] = 255;
            }
        }

        glTextureSubImage3D(_halfvec_lut, 0, 0, 0, face, SIZE, SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    }
}
