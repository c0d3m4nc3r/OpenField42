#include "geometry/material.h"
#include "assets/shader.h"
#include "assets/texture.h"
#include "core/log.h"
#include "vfs/vfs.h"
#include "utils/string_utils.h"

#include "glad/glad.h"

#include <sstream>
#include <vector>

void Material::apply(Shader* shader)
{
    if (!shader) return;

    shader->setVec4("uMaterial.diffuseColor", diffuse_color.toVec4());
    shader->setVec3("uMaterial.specularColor", specular_color.toVec3());
    shader->setFloat("uMaterial.specularPower", specular_power);

    shader->setBool("uMaterial.lighting", lighting);
    shader->setBool("uMaterial.lightingSpecular", lighting_specular);

    if (texture)
    {
        texture->bind(0);
        shader->setInt("uMaterial.texture", 0);
        shader->setBool("uMaterial.hasTexture", true);
    } else {
        shader->setBool("uMaterial.hasTexture", false);
    }

    if (detail_texture)
    {
        detail_texture->bind(1);
        shader->setInt("uMaterial.detailTexture", 1);
        shader->setBool("uMaterial.hasDetailTexture", true);
    } else {
        shader->setBool("uMaterial.hasDetailTexture", false);
    }

    if (twosided) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
    }
}

std::vector<Material> Material::load(const std::string& path)
{
    auto data = VFS::readFileString(path);
    if (data.empty())
    {
        LOG_ERROR("Material::load: Failed to read data from file '%s'!", path.c_str());
        return {};
    }

    std::istringstream stream(data);
    std::string line;
    int i = 0;

    std::vector<Material> materials;
    Material current;

    while (std::getline(stream, line))
    {
        i++;

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
                    materials.push_back(std::move(current));
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
                auto texture = Texture::load(tokens[1]);
                if (!texture)
                {
                    LOG_WARNING("Material::load: Failed to load texture from '%s' for material '%s'!",
                        tokens[1].c_str(), current.name.c_str());
                } else {
                    current.transparent = texture->isTransparent();
                    current.texture = texture;
                }
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
        materials.push_back(std::move(current));
    }

    return materials;
}
