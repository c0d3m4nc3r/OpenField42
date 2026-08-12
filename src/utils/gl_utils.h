#pragma once

namespace GLUtils
{
    unsigned int createTexture2D(
        int width, int height,
        GLenum internal_format, GLenum format,
        GLenum type,
        const void* data = nullptr,
        bool generate_mipmaps = false
    );
    unsigned int compileShader(const char* src, unsigned int type);
    unsigned int linkProgram(unsigned int vert_shader, unsigned int frag_shader);
}