#include "render/shader.h"

#include "glad/gl.h"

#include <glm/gtc/type_ptr.hpp>

Shader::~Shader()
{
    glDeleteProgram(_id);
}

void Shader::use()
{
    static unsigned int current_bound_id = 0;
    
    if (current_bound_id == _id) return;
    glUseProgram(_id);
    current_bound_id = _id;
}

void Shader::rebindUniformBlocks()
{
    for (const auto& [name, binding] : _ubo_bindings) 
    {
        GLuint index = glGetUniformBlockIndex(_id, name.c_str());
        if (index != GL_INVALID_INDEX) {
            glUniformBlockBinding(_id, index, binding);
        }
    }
}

void Shader::setUniformBlockBinding(const char* name, uint32_t binding)
{
    GLuint block_index = glGetUniformBlockIndex(_id, name);
    if (block_index == GL_INVALID_INDEX)
    {
        LOG_ERROR("Shader::bindUBO: Uniform block '%s' not found in shader %u!", name, _id);
        return;
    }

    glUniformBlockBinding(_id, block_index, binding);

    _ubo_bindings[name] = binding;
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
