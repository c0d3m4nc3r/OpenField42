#include "render/texture.h"
#include "utils/gl_utils.h"
#include "utils/log.h"

#include "utils/texture_utils.h"

#include <cmath>
#include <cstddef>

Texture::Texture(unsigned int id) : _id(id) {}
Texture::~Texture()
{
    if (_id != 0) glDeleteTextures(1, &_id);
}

std::shared_ptr<Texture> Texture::load(const std::string& path, bool generate_mipmaps)
{
    int width, height, channels;
    auto texture_data = TextureUtils::loadData(path, &width, &height, &channels);
    if (texture_data.empty())
    {
        LOG_ERROR("Texture::load: Failed to load texture from '%s'!", path.c_str());
        return nullptr;
    }

    GLuint texture = GLUtils::createTexture2D(
        width, height,
        TextureUtils::getInternalFormat(channels),
        TextureUtils::getFormat(channels),
        GL_UNSIGNED_BYTE,
        texture_data.data(),
        generate_mipmaps
    );
    
    auto result = std::make_shared<Texture>(texture);
    result->_transparent = channels == 4 ? true : false;
    return result;
}

std::shared_ptr<Texture> Texture::loadAtlas(const std::vector<std::string>& paths, int tile_w, int tile_h, int channels, bool generate_mipmaps)
{
    if (paths.empty())
    {
        LOG_ERROR("Texture::loadAtlas: No paths provided!");
        return nullptr;
    }

    size_t tiles_count = paths.size();
    size_t atlas_cols = static_cast<size_t>(std::ceil(std::sqrt(tiles_count)));
    size_t atlas_rows = static_cast<size_t>(std::ceil(tiles_count / (float)atlas_cols));

    int atlas_w = atlas_cols * tile_w;
    int atlas_h = atlas_rows * tile_h;

    std::vector<unsigned char> atlas_data(atlas_w * atlas_h * channels, 0);

    for (size_t i = 0; i < tiles_count; ++i)
    {
        int x = (i % atlas_cols) * tile_w;
        int y = (i / atlas_cols) * tile_h;

        int original_w, original_h, original_channels;
        auto tile_data = TextureUtils::loadData(paths[i], &original_w, &original_h, &original_channels);
        if (tile_data.empty())
        {
            LOG_ERROR("Texture::loadAtlas: Failed to load texture from '%s'!", paths[i].c_str());
            continue;
        }

        // if (original_channels != channels)
        // {
        //     LOG_WARNING("Texture::loadAtlas: Channels mismatch for '%s'. Expected: %d, got: %d. Texture may look incorrect.", 
        //                paths[i].c_str(), channels, original_channels);
        // }

        for (int ty = 0; ty < tile_h; ++ty)
        {
            for (int tx = 0; tx < tile_w; ++tx)
            {
                int src_x = tx % original_w;
                int src_y = ty % original_h;
                
                for (int c = 0; c < channels; ++c)
                {
                    int src_channel = std::min(c, original_channels - 1);
                    atlas_data[channels * ((y + ty) * atlas_w + (x + tx)) + c] =
                        tile_data[original_channels * (src_y * original_w + src_x) + src_channel];
                }
            }
        }
    }

    GLuint texture = GLUtils::createTexture2D(
        atlas_w, atlas_h,
        TextureUtils::getInternalFormat(channels),
        TextureUtils::getFormat(channels),
        GL_UNSIGNED_BYTE,
        atlas_data.data(),
        generate_mipmaps
    );

    return std::make_shared<Texture>(texture);
}

void Texture::bind(int slot)
{
    if (slot < 0 || slot >= 32) return;
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, _id);
}

void Texture::unbind(int slot)
{
    if (slot < 0 || slot >= 32) return;
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}
