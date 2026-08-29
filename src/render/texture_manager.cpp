#include "render/texture_manager.h"

#include "core/globals.h"
#include "utils/gl_utils.h"
#include "utils/texture_utils.h"

#include <algorithm>
#include <cmath>

TextureHandle TextureManager::load(const std::string& path)
{
    auto it = _path_to_handle.find(path);
    if (it != _path_to_handle.end())
        return it->second;

    TextureHandle new_handle;
    new_handle.id = static_cast<unsigned int>(_textures.size());

    getDefault();
    _textures.push_back(_default_tex);
    _path_to_handle[path] = new_handle;

    g_ThreadPool.enqueue([this, path, new_handle]() {
        TextureData data = TextureUtils::loadData(path);
        if (data.is_valid)
        {
            data.handle = new_handle;
            data.path = path;
            _completed_uploads.push(std::move(data));
        }
        else
        {
            LOG_ERROR("TextureManager::load: Failed to load texture from '%s'!", path.c_str());
        }
    });

    return new_handle;
}

std::vector<unsigned char> TextureManager::conformTileData(const TextureData& src, int target_w, int target_h, int target_channels)
{
    std::vector<unsigned char> out(static_cast<size_t>(target_w) * target_h * target_channels);

    for (int ty = 0; ty < target_h; ++ty)
    {
        int src_y = ty % src.height;
        for (int tx = 0; tx < target_w; ++tx)
        {
            int src_x = tx % src.width;
            for (int c = 0; c < target_channels; ++c)
            {
                int src_channel = std::min(c, src.channels - 1);
                out[target_channels * (ty * target_w + tx) + c] =
                    src.pixels[src.channels * (src_y * src.width + src_x) + src_channel];
            }
        }
    }

    return out;
}

int TextureManager::calcMipLevels(int w, int h)
{
    return 1 + static_cast<int>(std::floor(std::log2(static_cast<double>(std::max(w, h)))));
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

    int atlas_w = static_cast<int>(atlas_cols) * tile_w;
    int atlas_h = static_cast<int>(atlas_rows) * tile_h;
    int mip_levels = calcMipLevels(atlas_w, atlas_h);

    GLuint atlas_gl_id = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &atlas_gl_id);
    glTextureStorage2D(atlas_gl_id, mip_levels, TextureUtils::getInternalFormat(channels), atlas_w, atlas_h);

    glTextureParameteri(atlas_gl_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(atlas_gl_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    TextureHandle handle = { static_cast<unsigned int>(_textures.size()) };
    _textures.push_back(std::make_shared<Texture>(atlas_gl_id, channels == 4));

    _atlas_pending_tiles[handle.id] = static_cast<int>(paths.size());

    for (size_t i = 0; i < paths.size(); ++i)
    {
        int x = static_cast<int>(i % atlas_cols) * tile_w;
        int y = static_cast<int>(i / atlas_cols) * tile_h;

        const std::string& path = paths[i];

        g_ThreadPool.enqueue([this, path, handle, x, y, tile_w, tile_h, channels]() {
            TextureData tile_src = TextureUtils::loadData(path);
            if (!tile_src.is_valid)
            {
                LOG_ERROR("TextureManager::loadAtlas: Failed to load texture from '%s'!", path.c_str());
                return;
            }

            TextureData task;
            task.pixels = conformTileData(tile_src, tile_w, tile_h, channels);
            task.width = tile_w;
            task.height = tile_h;
            task.channels = channels;
            task.is_valid = true;
            task.path = path;

            task.handle = handle;
            task.is_atlas_tile = true;
            task.dst_x = x;
            task.dst_y = y;

            _completed_uploads.push(std::move(task));
        });
    }

    return handle;
}

void TextureManager::clear()
{
    size_t count = _textures.size();

    _path_to_handle.clear();
    _textures.clear();
    _atlas_pending_tiles.clear();
    _memory_usage = 0;

    LOG_INFO("TextureManager::clear: All %zu textures unloaded!", count);
}

void TextureManager::uploadAtlasTile(const TextureData& task)
{
    auto& atlas_tex = get(task.handle);
    GLuint atlas_gl_id = atlas_tex.getID();

    glTextureSubImage2D(
        atlas_gl_id, 0,
        task.dst_x, task.dst_y,
        task.width, task.height,
        TextureUtils::getFormat(task.channels),
        GL_UNSIGNED_BYTE,
        task.pixels.data()
    );

    auto pending_it = _atlas_pending_tiles.find(task.handle.id);
    if (pending_it != _atlas_pending_tiles.end())
    {
        if (--pending_it->second <= 0)
        {
            glGenerateTextureMipmap(atlas_gl_id);
            _atlas_pending_tiles.erase(pending_it);
        }
    }

    _memory_usage += task.pixels.size();
}

void TextureManager::uploadNewTexture(const TextureData& task)
{
    GLuint texture_id = GLUtils::createTexture2D(
        task.width, task.height,
        TextureUtils::getInternalFormat(task.channels),
        TextureUtils::getFormat(task.channels),
        GL_UNSIGNED_BYTE,
        task.pixels.data(),
        true
    );

    _textures[task.handle.id] = std::make_shared<Texture>(texture_id, task.channels == 4);
    _memory_usage += task.pixels.size();
}

void TextureManager::updateGpuUploads(int max_uploads_per_frame)
{
    int uploaded = 0;

    while (uploaded < max_uploads_per_frame)
    {
        auto task_opt = _completed_uploads.pop();
        if (!task_opt.has_value()) break;

        const auto& task = *task_opt;

        if (task.is_atlas_tile)
            uploadAtlasTile(task);
        else
            uploadNewTexture(task);

        uploaded++;
    }
}

Texture& TextureManager::get(const TextureHandle& handle)
{
    if (!handle.isValid() || handle.id >= _textures.size())
        return getDefault();
    return *_textures.at(handle.id);
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

        _default_tex = std::make_shared<Texture>(texture);
    }

    return *_default_tex;
}