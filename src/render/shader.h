#pragma once

#include <glm/fwd.hpp>

class Shader
{
public:

    Shader(GLuint id) : _id(id) {}
    ~Shader();

    void use();
    
    void setID(GLuint id) { _id = id; }
    GLuint getID() const { return _id; }

    void setInt(const char* name, int value);
    void setFloat(const char* name, float value);
    void setBool(const char* name, bool value);
    void setVec2(const char* name, const glm::vec2& value);
    void setVec3(const char* name, const glm::vec3& value);
    void setVec4(const char* name, const glm::vec4& value);
    void setMat3(const char* name, const glm::mat3& value);
    void setMat4(const char* name, const glm::mat4& value);

private:

    GLuint _id;

    std::unordered_map<std::string, int> _uniform_locations;

    int getUniformLocation(const char* name);

};
