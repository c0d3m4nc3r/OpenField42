#include "render/texture_manager.h"

#include "render/texture.h"
#include "utils/gl_utils.h"
#include "utils/texture_utils.h"

TextureHandle TextureManager::load(const std::string& path)
{
    auto it = _path_to_handle.find(path);
    if (it != _path_to_handle.end())
        return it->second;
    
    LOG_INFO("TextureManager::load: Loading texture from '%s'...", path.c_str());

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

    TextureHandle handle = { static_cast<unsigned int>(_textures.size()) };
    _path_to_handle[path] = handle;
    _textures.emplace_back(texture);

    LOG_INFO("TextureManager::load: Texture '%s' loaded! (ID: %u)", path.c_str(), handle.id);

    return handle;
}

void TextureManager::clear()
{
    size_t count = _textures.size();

    _path_to_handle.clear();
    _textures.clear();

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