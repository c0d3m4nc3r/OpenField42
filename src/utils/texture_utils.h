#pragma once

#include "render/texture.h"

#include "glad/glad.h"

#include <vector>

namespace TextureUtils
{
    GLuint createGLTexture(int width, int height, GLenum internal_format, GLenum format, GLenum type, const void* data = nullptr);
    std::vector<unsigned char> loadData(const std::string& path, int* out_width, int* out_height, int* out_channels);
    GLenum getFormat(int channels);
    GLenum getInternalFormat(int channels);
}
