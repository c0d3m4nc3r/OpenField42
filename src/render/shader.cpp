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

#define IMPLEMENT_SET_UNIFORM(METHOD_NAME, TYPE, FUNC, ...) \
void Shader::METHOD_NAME(const char* name, TYPE value)       \
{                                                            \
    int location = getUniformLocation(name);                 \
    if (location != -1)                                      \
    {                                                        \
        FUNC(_id, location, __VA_ARGS__);                    \
    }                                                        \
}

IMPLEMENT_SET_UNIFORM(setInt, int, glProgramUniform1i, value)
IMPLEMENT_SET_UNIFORM(setFloat, float, glProgramUniform1f, value)
IMPLEMENT_SET_UNIFORM(setBool, bool, glProgramUniform1i, static_cast<int>(value))
IMPLEMENT_SET_UNIFORM(setVec2, const glm::vec2&, glProgramUniform2fv, 1, glm::value_ptr(value))
IMPLEMENT_SET_UNIFORM(setVec3, const glm::vec3&, glProgramUniform3fv, 1, glm::value_ptr(value))
IMPLEMENT_SET_UNIFORM(setVec4, const glm::vec4&, glProgramUniform4fv, 1, glm::value_ptr(value))
IMPLEMENT_SET_UNIFORM(setMat3, const glm::mat3&, glProgramUniformMatrix3fv, 1, GL_FALSE, glm::value_ptr(value))
IMPLEMENT_SET_UNIFORM(setMat4, const glm::mat4&, glProgramUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(value))

#undef IMPLEMENT_SET_UNIFORM

int Shader::getUniformLocation(const char* name)
{
    auto it = _uniform_locations.find(name);
    if (it != _uniform_locations.end())
        return it->second;

    int location = glGetUniformLocation(_id, name);
    if (location == -1)
    {
        LOG_WARNING("Shader::getUniformLocation: Location '%s' not found in shader program %u", name, _id);
    }

    _uniform_locations[name] = location;
    return location;
}