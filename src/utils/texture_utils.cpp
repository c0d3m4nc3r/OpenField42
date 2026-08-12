#include "utils/texture_utils.h"
#include "utils/log.h"
#include "vfs/vfs.h"

#include "SOIL2.h"

#include <cstring>

namespace TextureUtils
{
    std::vector<unsigned char> loadData(const std::string& path, int* out_width, int* out_height, int* out_channels)
    {
        std::vector<char> file_data = VFS::readFileData(path);
        if (file_data.empty())
        {
            LOG_ERROR("TextureUtils::loadData: Failed to read file '%s'!", path.c_str());
            return {};
        }

        int width = 0, height = 0, channels = 0;
        unsigned char* data = nullptr;
        bool is_custom_alloc = false;

        // NOTE: Hack for RGB565 dds images which is used for normal maps in bf1942
        if (file_data.size() >= 128 && 
            file_data[0] == 'D' && file_data[1] == 'D' && file_data[2] == 'S' && file_data[3] == ' ')
        {
            uint32_t bit_count = *reinterpret_cast<const uint32_t*>(&file_data[4 + 84]);
            uint32_t r_mask    = *reinterpret_cast<const uint32_t*>(&file_data[4 + 88]);

            if (bit_count == 16 && r_mask == 0xF800)
            {
                width    = *reinterpret_cast<const uint32_t*>(&file_data[4 + 12]);
                height   = *reinterpret_cast<const uint32_t*>(&file_data[4 + 8]);
                channels = 4;

                size_t num_pixels = static_cast<size_t>(width) * height;
                size_t expected_file_size = 128 + num_pixels * 2;

                if (file_data.size() >= expected_file_size)
                {
                    data = static_cast<unsigned char*>(malloc(num_pixels * 4));
                    if (data)
                    {
                        is_custom_alloc = true;
                        const uint16_t* raw_pixels = reinterpret_cast<const uint16_t*>(&file_data[128]);

                        for (size_t i = 0; i < num_pixels; ++i)
                        {
                            uint16_t pixel = raw_pixels[i];
                            
                            uint8_t r = (pixel >> 11) & 0x1F;
                            uint8_t g = (pixel >> 5)  & 0x3F;
                            uint8_t b =  pixel        & 0x1F;

                            data[i * 4 + 0] = (r << 3) | (r >> 2);
                            data[i * 4 + 1] = (g << 2) | (g >> 4);
                            data[i * 4 + 2] = (b << 3) | (b >> 2);
                            data[i * 4 + 3] = 255;
                        }
                    }
                }
            }
        }
        if (!data)
        {
            data = SOIL_load_image_from_memory(
                reinterpret_cast<const unsigned char*>(file_data.data()),
                static_cast<int>(file_data.size()),
                &width, &height, &channels,
                SOIL_LOAD_AUTO
            );
        }

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

        if (is_custom_alloc)
            free(data);
        else
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
