#pragma once

#include <glm/fwd.hpp>

class Shader
{
public:

    Shader(unsigned int id) : _id(id) {}
    ~Shader();

    void use();
    
    void setID(unsigned int id) { _id = id; }
    unsigned int getID() const { return _id; }

    void rebindUniformBlocks();
    void setUniformBlockBinding(const char* name, uint32_t binding);

    void setInt(const char* name, int value);
    void setFloat(const char* name, float value);
    void setBool(const char* name, bool value);
    void setVec2(const char* name, const glm::vec2& value);
    void setVec3(const char* name, const glm::vec3& value);
    void setVec4(const char* name, const glm::vec4& value);
    void setMat3(const char* name, const glm::mat3& value);
    void setMat4(const char* name, const glm::mat4& value);

private:

    unsigned int _id;

    std::unordered_map<std::string, uint32_t> _ubo_bindings;
};
