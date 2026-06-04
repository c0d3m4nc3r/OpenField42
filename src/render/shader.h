#pragma once

#include <glm/fwd.hpp>

#include <string>

class Shader
{
public:

    Shader() = default;
    ~Shader();

    bool load(
        const std::string& vert_path,
        const std::string& frag_path
    );

    void unload();
    void use();

    unsigned int getID() const { return _id; }

    void setInt(const char* name, int value);
    void setFloat(const char* name, float value);
    void setBool(const char* name, bool value);
    void setVec2(const char* name, const glm::vec2& value);
    void setVec3(const char* name, const glm::vec3& value);
    void setVec4(const char* name, const glm::vec4& value);
    void setMat3(const char* name, const glm::mat3& value);
    void setMat4(const char* name, const glm::mat4& value);

private:

    unsigned int _id = 0;
};
