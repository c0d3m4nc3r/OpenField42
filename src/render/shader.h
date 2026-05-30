#pragma once

#include <glm/fwd.hpp>

#include <memory>
#include <string>

class Shader
{
public:

    Shader(unsigned int id);
    ~Shader();

    static std::shared_ptr<Shader> load(
        const std::string& vs_path,
        const std::string& fs_path
    );

    void bind();
    void unbind();

    void setInt(const char* name, int value);
    void setFloat(const char* name, float value);
    void setBool(const char* name, bool value);
    void setVec2(const char* name, const glm::vec2& value);
    void setVec3(const char* name, const glm::vec3& value);
    void setVec4(const char* name, const glm::vec4& value);
    void setMat3(const char* name, const glm::mat3& value);
    void setMat4(const char* name, const glm::mat4& value);

private:

    unsigned int id;

    static unsigned int current_bound_id;
};
