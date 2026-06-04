#include "render/shader.h"
#include "utils/log.h"
#include "vfs/vfs.h"

#include "glad/glad.h"

#include <glm/gtc/type_ptr.hpp>

#include <vector>

static GLuint compileShader(const std::string& src, GLenum type);
static GLuint linkProgram(GLuint vertex_shader, GLuint fragment_shader);

Shader::~Shader()
{
    unload();
}

bool Shader::load(const std::string& vs_path, const std::string& fs_path)
{
    LOG_INFO("Shader::load: Loading shader from '%s' and '%s'...", vs_path.c_str(), fs_path.c_str());

    std::string vertex_src = VFS::readFileString(vs_path);
    std::string fragment_src = VFS::readFileString(fs_path);

    if (vertex_src.empty() || fragment_src.empty())
    {
        LOG_ERROR("Shader::load: Failed to read shader sources!");
        return false;
    }

    GLuint vertex_shader = compileShader(vertex_src, GL_VERTEX_SHADER);
    GLuint fragment_shader = compileShader(fragment_src, GL_FRAGMENT_SHADER);

    if (!vertex_shader || !fragment_shader)
    {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return false;
    }

    _id = linkProgram(vertex_shader, fragment_shader);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    if (!_id) return false;

    LOG_INFO("Shader::load: Shader loaded! (ID: %u)", _id);

    return true;
}

void Shader::unload()
{
    if (!_id) return;

    glDeleteProgram(_id);
    LOG_INFO("Shader::unload: Shader unloaded! (ID: %u)", _id);
    _id = 0;
}

void Shader::use()
{
    static unsigned int current_bound_id = 0;
    
    if (current_bound_id == _id) return;
    glUseProgram(_id);
    current_bound_id = _id;
}

void Shader::setInt(const char* name, int value)
{
    glUniform1i(glGetUniformLocation(_id, name), value);
}

void Shader::setFloat(const char* name, float value)
{
    glUniform1f(glGetUniformLocation(_id, name), value);
}

void Shader::setBool(const char* name, bool value)
{
    glUniform1i(glGetUniformLocation(_id, name), static_cast<int>(value));
}

void Shader::setVec2(const char* name, const glm::vec2& value)
{
    glUniform2fv(glGetUniformLocation(_id, name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const char* name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(_id, name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const char* name, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(_id, name), 1, glm::value_ptr(value));
}

void Shader::setMat3(const char* name, const glm::mat3& value)
{
    glUniformMatrix3fv(glGetUniformLocation(_id, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const char* name, const glm::mat4& value)
{
    glUniformMatrix4fv(glGetUniformLocation(_id, name), 1, GL_FALSE, glm::value_ptr(value));
}

static GLuint compileShader(const std::string& src, GLenum type)
{
    GLuint shader = glCreateShader(type);
    const char* src_cstr = src.c_str();
    glShaderSource(shader, 1, &src_cstr, nullptr);
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
        LOG_ERROR("compileShader: Failed to compile %s shader: %s", type_str, log.data());
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint linkProgram(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLint log_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
        std::vector<char> log(log_len);
        glGetProgramInfoLog(program, log_len, nullptr, log.data());
        LOG_ERROR("linkProgram: Failed to link shader program: %s", log.data());
        glDeleteProgram(program);
        return 0;
    }

    return program;
}
