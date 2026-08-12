#include "utils/gl_utils.h"

#include "glad/gl.h"

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
