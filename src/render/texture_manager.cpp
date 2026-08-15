#include "render/texture_manager.h"

#include "utils/gl_utils.h"
#include "utils/texture_utils.h"

TextureHandle TextureManager::load(const std::string& path)
{
    auto it = _path_to_handle.find(path);
    if (it != _path_to_handle.end())
        return it->second;

    int width, height, channels;
    auto texture_data = TextureUtils::loadData(path, &width, &height, &channels);
    if (texture_data.empty())
    {
        LOG_ERROR("TextureManager::load: Failed to load texture from '%s'!", path.c_str());
        return { INVALID_TEXTURE_ID };
    }

    GLuint texture = GLUtils::createTexture2D(
        width, height,
        TextureUtils::getInternalFormat(channels),
        TextureUtils::getFormat(channels),
        GL_UNSIGNED_BYTE,
        texture_data.data(),
        true // todo
    );

    TextureHandle handle = { static_cast<unsigned int>(_textures.size() ) };
    _path_to_handle[path] = handle;
    _textures.emplace_back(texture, channels == 4 ? true : false);
    _memory_usage += texture_data.size();

    LOG_DEBUG("TextureManager::load: Texture '%s' loaded! (ID: %u, Width: %d, Height: %d, Size: %zu KB)",
        path.c_str(), handle.id, width, height, texture_data.size() / 1024);

    return handle;
}

TextureHandle TextureManager::loadAtlas(const std::vector<std::string>& paths, int tile_w, int tile_h, int channels)
{
    if (paths.empty())
    {
        LOG_ERROR("TextureManager::loadAtlas: No paths provided!");
        return { INVALID_TEXTURE_ID };
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
            LOG_ERROR("TextureManager::loadAtlas: Failed to load texture from '%s'!", paths[i].c_str());
            continue;
        }

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
        true // todo
    );

    TextureHandle handle = { static_cast<unsigned int>(_textures.size() ) };
    _textures.emplace_back(texture, channels == 4 ? true : false);
    _memory_usage += atlas_data.size();

    LOG_DEBUG("TextureManager::loadAtlas: Texture atlas loaded from %zu paths! (Size: %zu KB, ID: %u)",
        paths.size(), atlas_data.size() / 1024, handle.id);

    for (const auto& path : paths)
    {
        LOG_DEBUG("TextureManager::loadAtlas: - %s", path.c_str());
    }

    return handle;
}

void TextureManager::clear()
{
    size_t count = _textures.size();

    _path_to_handle.clear();
    _textures.clear();
    _memory_usage = 0;

    LOG_INFO("TextureManager::clear: All %zu textures unloaded!", count);
}

Texture& TextureManager::get(const TextureHandle& handle)
{
    if (!handle.isValid()) return getDefault();
    return _textures.at(handle.id);
}

Texture& TextureManager::getDefault()
{
    if (!_default_tex)
    {
        GLuint texture;
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);
        glTextureStorage2D(texture, 1, GL_RGBA8, 2, 2);
        
        const unsigned char pixels[] = {
            255, 0, 255, 255, 0, 0, 0, 255,
            0, 0, 0, 255, 255, 0, 255, 255
        };

        glTextureSubImage2D(texture, 0, 0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        _default_tex = std::make_unique<Texture>(texture);
    }

    return *_default_tex.get();
}