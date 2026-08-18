#include "geometry/standard_mesh.h"

#include "core/buffer_reader.h"
#include "core/globals.h"
#include "geometry/material.h"
#include "geometry/geometry_template.h"
#include "utils/log.h"
#include "vfs/vfs.h"
#include "utils/string_utils.h"

#include <sstream>
#include <cstring>

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

    auto bytes = g_VFS->readFile(full_path);
    if (bytes.empty())
    {
        LOG_ERROR("StandardMesh::load: Failed to read data from '%s'!", full_path.c_str());
        return false;
    }

    BufferReader reader(bytes);

    // 1. Materials

    if (!loadMaterials(tmpl))
    {
        LOG_ERROR("StandardMesh::load: Failed to load materials for '%s'!", tmpl->file.c_str());
        return false;
    }

    // 2. Header

    uint32_t version = reader.read<uint32_t>();
    
    reader.skip(sizeof(uint32_t));

    aabb.min = reader.read<glm::vec3>();
    aabb.max = reader.read<glm::vec3>();

    if (version > 9)
    {
        reader.skip(sizeof(uint8_t));
    }

    // 3. Collision meshes

    uint32_t col_mesh_num = reader.read<uint32_t>();

    // skip collision meshes
    for (uint32_t i = 0; i < col_mesh_num; i++)
    {
        uint32_t block_size = reader.read<uint32_t>();
        reader.skip(block_size);
    }

    // 4. LODs

    uint32_t lod_num = reader.read<uint32_t>();
    lods.resize(lod_num);

    for (uint32_t i = 0; i < lod_num; i++)
    {
        auto& lod = lods[i];

        uint32_t mesh_num = reader.read<uint32_t>();
        lod.meshes.resize(mesh_num);

        struct MeshInfo
        {
            uint32_t vertex_count;
            uint32_t index_count;
            uint32_t vertex_stride;
        };

        std::vector<MeshInfo> mesh_infos;
        mesh_infos.reserve(mesh_num);

        uint32_t total_vertices = 0;
        uint32_t total_indices = 0;

        for (uint32_t j = 0; j < mesh_num; j++)
        {
            Mesh& mesh = lods[i].meshes[j];

            uint32_t mat_name_len = reader.read<uint32_t>();
            std::string mat_name = reader.readString(mat_name_len);

            auto it = materials.find(StringUtils::lowercase(mat_name));
            if (it != materials.end()) {
                mesh.material = &it->second;
            } else {
                LOG_WARNING("StandardMesh::load: Material '%s' not found for '%s'!", mat_name.c_str(), tmpl->name.c_str());
                mesh.material = nullptr;
            }

            reader.skip(sizeof(uint32_t) * 3);

            uint32_t prim_type = reader.read<uint32_t>();
            assert(prim_type == 4);

            reader.skip(sizeof(uint32_t));

            auto& info = mesh_infos.emplace_back();

            info.vertex_stride = reader.read<uint32_t>();
            info.vertex_count = reader.read<uint32_t>();
            info.index_count = reader.read<uint32_t>();

            total_vertices += info.vertex_count;
            total_indices += info.index_count;

            reader.skip(sizeof(uint32_t));
        }

        lod.vertices.reserve(total_vertices);
        lod.indices.reserve(total_indices);

        for (uint32_t j = 0; j < mesh_num; j++)
        {
            auto& mesh = lod.meshes[j];
            auto& info = mesh_infos[j];

            mesh.index_count = info.index_count;
            mesh.index_start = (uint32_t)lod.indices.size();
            mesh.base_vertex = (uint32_t)lod.vertices.size();

            for (uint32_t v = 0; v < info.vertex_count; v++)
            {
                Vertex vertex;
                vertex.position = reader.read<glm::vec3>();
                vertex.normal = reader.read<glm::vec3>();
                vertex.uv = reader.read<glm::vec2>();
                vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};

                lod.vertices.push_back(vertex);
                
                size_t extra = info.vertex_stride - (sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec2));
                reader.skip(extra);
            }

            for (uint32_t idx = 0; idx < info.index_count; idx++)
            {
                uint16_t raw = reader.read<uint16_t>();
                lod.indices.push_back(raw);
            }
        }
    }

    LOG_DEBUG("StandardMesh::load: Loaded '%s' with %zu materials and %zu LODs!", tmpl->name.c_str(), materials.size(), lods.size());
    
    return true;
}

bool StandardMesh::loadMaterials(const GeometryTemplate* tmpl)
{
    std::string full_path = "standardMesh/" + tmpl->file + ".rs";

    auto data = g_VFS->readFileString(full_path);
    if (data.empty())
    {
        LOG_ERROR("StandardMesh::loadMaterials: Failed to read data from file '%s'!", full_path.c_str());
        return false;
    }

    std::istringstream stream(data);
    std::string line;

    Material current;

    while (std::getline(stream, line))
    {
        if (line.empty()) continue;

        std::erase_if(line, [](char c) {
            return c == '\t' || c == '\r' || c == '\"' || c == '\'' || c == ';';
        });

        auto tokens = StringUtils::split(line);

        if (tokens.empty()) continue;

        // Create new material
        if (tokens[0] == "subshader")
        {
            if (tokens.size() >= 3)
            {
                if (!current.name.empty())
                {
                    materials[current.name] = std::move(current);
                }
    
                current = Material{};
                current.name = StringUtils::lowercase(tokens[1]);
            }
        }
        // Strings
        else if (tokens[0] == "texture")
        {
            if (tokens.size() >= 2)
            {
                auto texture = g_TextureMgr->load(tokens[1]);
                if (!texture.isValid())
                {
                    LOG_WARNING("Material::load: Failed to load texture from '%s' for material '%s'!",
                        tokens[1].c_str(), current.name.c_str());
                }
                current.texture = texture;
            }
        }
        // Colors
        else if (tokens[0] == "materialDiffuse")
        {
            if (tokens.size() >= 4)
            {
                current.diffuse_color = Color(
                    std::stof(tokens[1]),
                    std::stof(tokens[2]),
                    std::stof(tokens[3])
                );
            }
        }
        else if (tokens[0] == "materialSpecular")
        {
            if (tokens.size() >= 4)
            {
                current.specular_color = Color(
                    std::stof(tokens[1]),
                    std::stof(tokens[2]),
                    std::stof(tokens[3])
                );
            }
        }
        // Floats
        else if (tokens[0] == "materialSpecularPower")
        {
            if (tokens.size() >= 2)
            {
                current.specular_power = std::stof(tokens[1]);
            }
        }
        // Booleans
        else if (tokens[0] == "lighting") {
            current.lighting = (tokens.size() > 1 && tokens[1] == "true");
        } else if (tokens[0] == "lightingSpecular") {
            current.lighting_specular = (tokens.size() > 1 && tokens[1] == "true");
        } else if (tokens[0] == "twosided") {
            current.twosided = (tokens.size() > 1 && tokens[1] == "true");
        }
        // NOTE: Lie
        // else if (tokens[0] == "transparent") {
        //     current.transparent = (tokens.size() > 1 && tokens[1] == "true");
        // }
    }

    if (!current.name.empty())
    {
        materials[current.name] = std::move(current);
    }

    return true;
}
