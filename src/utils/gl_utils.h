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
    unsigned int compileShader(GLenum type, const char** src, int count);
    unsigned int linkProgram(GLuint vert_shader, GLuint frag_shader);
}