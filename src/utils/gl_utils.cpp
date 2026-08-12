#include "utils/gl_utils.h"

#include "core/config.h"

#include "glad/gl.h"

unsigned int GLUtils::createTexture2D(
    int width, int height,
    GLenum internal_format, GLenum format,
    GLenum type,
    const void* data,
    bool generate_mipmaps
)
{
    GLuint texture = 0;

    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    int levels = 1;
    if (generate_mipmaps && width > 0 && height > 0)
    {
        int maxdim = std::max(width, height);
        levels = static_cast<int>(std::floor(std::log2(static_cast<float>(maxdim)))) + 1;
    }

    glTextureStorage2D(texture, levels, internal_format, width, height);

    if (data)
        glTextureSubImage2D(texture, 0, 0, 0, width, height, format, type, data);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, DEFAULT_MIN_FILTER);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, DEFAULT_MAG_FILTER);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, DEFAULT_WRAP_S);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, DEFAULT_WRAP_T);

    if (generate_mipmaps)
        glGenerateTextureMipmap(texture);

    // LOG_DEBUG("GLUtils::createTexture2D: Created texture (ID: %u, Size: %dx%d, Levels: %d)", texture, width, height, levels);

    return texture;
}

unsigned int GLUtils::compileShader(const char* src, unsigned int type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLint log_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        std::vector<char> log(log_len);
        glGetShaderInfoLog(shader, log_len, nullptr, log.data());
        const char* type_str = type == GL_VERTEX_SHADER ? "vertex" : "fragment";
        LOG_ERROR("GLUtils::compileShader: Failed to compile %s shader: %s", type_str, log.data());
        glDeleteShader(shader);
        return 0;
    }

    LOG_DEBUG("GLUtils::compileShader: Shader compiled successfully! (ID: %u)", shader);

    return shader;
}

unsigned int GLUtils::linkProgram(unsigned int vert_shader, unsigned int frag_shader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint log_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
        std::vector<char> log(log_len);
        glGetProgramInfoLog(program, log_len, nullptr, log.data());
        LOG_ERROR("GLUtils::linkProgram: Failed to link shader program: %s", log.data());
        glDeleteProgram(program);
        return 0;
    }

    LOG_DEBUG("GLUtils::linkProgram: Program linked successfully! (ID: %u)", program);

    return program;
}
