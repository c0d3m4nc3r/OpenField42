#include "world/water.h"
#include "render/shader.h"
#include "render/texture.h"

Water g_Water;

bool Water::init(float size, float scale_xz, float height)
{
    _mesh.vertices.reserve(size * size);

    for (float z = 0.0f; z < size; z++)
    {
        for (float x = 0.0f; x < size; x++)
        {
            Geometry::Vertex v;
            v.position = {
                x * scale_xz,
                height,
                z * scale_xz
            };
    
            v.normal = {0.0f, 1.0f, 0.0f};
    
            v.uv.x = (x / (size - 1)) * 64.0f;
            v.uv.y = (z / (size - 1)) * 64.0f;
    
            _mesh.vertices.push_back(v);
        }
    }

    int i_size = (int)size;
    _mesh.indices.reserve((i_size - 1) * (i_size - 1) * 6);

    for (int z = 0; z < i_size - 1; z++)
    {
        for (int x = 0; x < i_size - 1; x++)
        {
            unsigned int v0 = z * i_size + x;
            unsigned int v1 = (z + 1) * i_size + x;
            unsigned int v2 = z * i_size + (x + 1);
            unsigned int v3 = (z + 1) * i_size + (x + 1);
    
            _mesh.indices.push_back(v0);
            _mesh.indices.push_back(v1);
            _mesh.indices.push_back(v2);
    
            _mesh.indices.push_back(v2);
            _mesh.indices.push_back(v1);
            _mesh.indices.push_back(v3);
        }
    }

    _mesh.upload();

    return true;
}

void Water::shutdown()
{
    _mesh.unload();
}

void Water::draw(Shader* shader)
{
    shader->setBool("uIsWater", true);

    if (g_Water.layers[0].texture)
    {
        g_Water.layers[0].texture->bind(0);
        shader->setInt("uWater.texLayer1", 0);
        shader->setVec2("uWater.scrollDir1", layers[0].scroll_dir);
        shader->setFloat("uWater.scrollSpeed1", layers[0].scroll_speed);
    }

    if (g_Water.layers[1].texture)
    {
        g_Water.layers[1].texture->bind(1);
        shader->setInt("uWater.texLayer2", 1);
        shader->setVec2("uWater.scrollDir2", layers[1].scroll_dir);
        shader->setFloat("uWater.scrollSpeed2", layers[1].scroll_speed);
    }
    
    shader->setVec4("uWater.shallowColor", shallow_color.toVec4());
    shader->setVec4("uWater.deepColor", deep_color.toVec4());
    shader->setFloat("uWater.alphaDepth", alpha_depth);

    shader->setVec3("uWireframeColor", glm::vec3(0.0f, 0.0f, 1.0f));

    shader->setMat4("uModel", glm::mat4(1.0f));

    _mesh.draw(shader);

    shader->setBool("uIsWater", false);
}