#include "utils/texture_utils.h"
#include "core/config.h"
#include "core/log.h"
#include "vfs/vfs.h"

#include "SOIL2.h"

#include <cstring>

namespace TextureUtils
{
    GLuint createGLTexture(int width, int height, GLenum internal_format, GLenum format, GLenum type, const void* data)
    {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, DEFAULT_MIN_FILTER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, DEFAULT_MAG_FILTER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, DEFAULT_WRAP_S);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, DEFAULT_WRAP_T);

        if (GENERATE_MIPMAPS)
            glGenerateMipmap(GL_TEXTURE_2D);
        
        glBindTexture(GL_TEXTURE_2D, 0);

        return texture;
    }

    std::vector<unsigned char> loadData(const std::string& path, int* out_width, int* out_height, int* out_channels)
    {
        std::vector<char> file_data = VFS::readFileData(path);
        if (file_data.empty())
        {
            LOG_ERROR("TextureUtils::loadData: Failed to read file '%s'!", path.c_str());
            return {};
        }

        int width, height, channels;
        unsigned char* data = SOIL_load_image_from_memory(
            reinterpret_cast<const unsigned char*>(file_data.data()),
            static_cast<int>(file_data.size()),
            &width, &height, &channels,
            SOIL_LOAD_AUTO
        );

        if (!data)
        {
            LOG_ERROR("TextureUtils::loadData: Failed to load data from '%s': %s!", path.c_str(), SOIL_last_result());
            return {};
        }

        if (out_width) *out_width = width;
        if (out_height) *out_height = height;
        if (out_channels) *out_channels = channels;

        size_t data_size = static_cast<size_t>(width) * height * channels;

        std::vector<unsigned char> result(data_size);
        memcpy(result.data(), data, data_size);

        SOIL_free_image_data(data);

        return result;
    }

    GLenum getFormat(int channels)
    {
        switch (channels)
        {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default:
            LOG_WARNING("TextureUtils::getFormat: Unsupported channel count: %d! Defaulting to 4...", channels);
            return GL_RGBA;
        }
    }

    GLenum getInternalFormat(int channels)
    {
        switch (channels)
        {
        case 1: return GL_R8;
        case 2: return GL_RG8;
        case 3: return GL_RGB8;
        case 4: return GL_RGBA8;
        default:
            LOG_ERROR("Texture::load: Unsupported channel count: %d! Defaulting to 4...", channels);
            return GL_RGBA8;
        }
    }
}
