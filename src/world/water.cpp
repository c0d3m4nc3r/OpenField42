#include "world/water.h"
#include "render/shader.h"
#include "render/texture.h"
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

    glGenBuffers(1, &_water_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, _water_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UBO_WaterBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

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

    glDeleteBuffers(1, &_water_ubo);

    _geometry.unload();

    LOG_INFO("Water::shutdown: Water shutdown");
}

void Water::draw(Shader* shader) const
{
    if (!shader) return;

    if (!_ubo_bound)
    {
        GLuint block_index = glGetUniformBlockIndex(shader->getID(), "WaterBlock");
        if (block_index != GL_INVALID_INDEX)
        {
            glUniformBlockBinding(shader->getID(), block_index, 3);
        }
        
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, _water_ubo);
        
        _ubo_bound = true;
    }

    if (_dirty)
    {
        UBO_WaterBlock water_data;
        water_data.layer_1 = {
            _layers[0].scroll_dir.x,
            _layers[0].scroll_dir.y,
            _layers[0].scroll_speed,
            _layers[0].uv_scale
        };

        water_data.layer_2 = {
            _layers[1].scroll_dir.x,
            _layers[1].scroll_dir.y,
            _layers[1].scroll_speed,
            _layers[1].uv_scale
        };

        glBindBuffer(GL_UNIFORM_BUFFER, _water_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UBO_WaterBlock), &water_data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        _dirty = false;
    }

    shader->use();

    if (_layers[0].texture)
    {
        _layers[0].texture->bind(0);
        shader->setInt("uTexLayer1", 0);
    }

    if (_layers[1].texture)
    {
        _layers[1].texture->bind(1);
        shader->setInt("uTexLayer2", 1);
    }

    _geometry.draw(shader, glm::mat4(1.0f));
}
