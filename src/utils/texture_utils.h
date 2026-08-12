#pragma once

#include "glad/gl.h"

#include <vector>

namespace TextureUtils
{
    std::vector<unsigned char> loadData(const std::string& path, int* out_width, int* out_height, int* out_channels);
    GLenum getFormat(int channels);
    GLenum getInternalFormat(int channels);
}
