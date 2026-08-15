#include "utils/texture_utils.h"
#include "utils/log.h"
#include "vfs/vfs.h"

#include "SOIL2.h"

#include <cstring>

namespace TextureUtils
{
    TextureData loadData(const std::string& path)
    {
        std::vector<char> file_data = VFS::readFileData(path);
        if (file_data.empty())
        {
            LOG_ERROR("TextureUtils::loadData: Failed to read file '%s'!", path.c_str());
            return {};
        }

        TextureData texture_data;
        unsigned char* pixels = nullptr;
        bool is_custom_alloc = false;

        // NOTE: Hack for RGB565 dds images which is used for normal maps in bf1942
        if (file_data.size() >= 128 && 
            file_data[0] == 'D' && file_data[1] == 'D' && file_data[2] == 'S' && file_data[3] == ' ')
        {
            uint32_t bit_count = *reinterpret_cast<const uint32_t*>(&file_data[4 + 84]);
            uint32_t r_mask    = *reinterpret_cast<const uint32_t*>(&file_data[4 + 88]);

            if (bit_count == 16 && r_mask == 0xF800)
            {
                texture_data.width    = *reinterpret_cast<const uint32_t*>(&file_data[4 + 12]);
                texture_data.height   = *reinterpret_cast<const uint32_t*>(&file_data[4 + 8]);
                texture_data.channels = 4;

                size_t num_pixels = static_cast<size_t>(texture_data.width) * texture_data.height;
                size_t expected_file_size = 128 + num_pixels * 2;

                if (file_data.size() >= expected_file_size)
                {
                    pixels = static_cast<unsigned char*>(malloc(num_pixels * 4));
                    if (pixels)
                    {
                        is_custom_alloc = true;
                        const uint16_t* raw_pixels = reinterpret_cast<const uint16_t*>(&file_data[128]);

                        for (size_t i = 0; i < num_pixels; ++i)
                        {
                            uint16_t pixel = raw_pixels[i];
                            
                            uint8_t r = (pixel >> 11) & 0x1F;
                            uint8_t g = (pixel >> 5)  & 0x3F;
                            uint8_t b =  pixel        & 0x1F;

                            pixels[i * 4 + 0] = (r << 3) | (r >> 2);
                            pixels[i * 4 + 1] = (g << 2) | (g >> 4);
                            pixels[i * 4 + 2] = (b << 3) | (b >> 2);
                            pixels[i * 4 + 3] = 255;
                        }
                    }
                }
            }
        }
        if (!pixels)
        {
            pixels = SOIL_load_image_from_memory(
                reinterpret_cast<const unsigned char*>(file_data.data()),
                static_cast<int>(file_data.size()),
                &texture_data.width, &texture_data.height, &texture_data.channels,
                SOIL_LOAD_AUTO
            );
        }

        if (!pixels)
        {
            LOG_ERROR("TextureUtils::loadData: Failed to load data from '%s': %s!", path.c_str(), SOIL_last_result());
            return {};
        }

        size_t data_size = static_cast<size_t>(texture_data.width) * texture_data.height * texture_data.channels;

        texture_data.pixels.resize(data_size);
        memcpy(texture_data.pixels.data(), pixels, data_size);

        texture_data.is_valid = true;

        if (is_custom_alloc)
            free(pixels);
        else
            SOIL_free_image_data(pixels);

        return texture_data;
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
