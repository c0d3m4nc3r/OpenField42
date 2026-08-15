#pragma once

#include "render/texture_manager.h"

class Texture;
struct Material;

struct RenderCommand
{
    uint32_t vao = 0;
    uint32_t index_count = 0;
    void* index_offset = 0;
    int base_vertex = 0;

    const Material* material = nullptr;
    uint32_t transform_id = 0;
    float distance_to_camera = 0.0f;

    TextureHandle textures[2]{};
};
