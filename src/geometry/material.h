#pragma once

#include <string>
#include <memory>

#include <glm/vec3.hpp>

struct Color
{
    float r, g, b, a;

    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}

    Color(float _r, float _g, float _b, float _a = 1.0f)
        : r(_r), g(_g), b(_b), a(_a) {}

    Color(const glm::vec3& vec)
        : r(vec.r), g(vec.g), b(vec.b), a(1.0f) {}

    Color(const glm::vec4& vec)
        : r(vec.r), g(vec.g), b(vec.b), a(vec.a) {}

    glm::vec3 toVec3() const { return glm::vec3(r, g, b); }
    glm::vec4 toVec4() const { return glm::vec4(r, g, b, a); }
};

class Texture;
class Shader;
struct Material
{
    std::string name = "";
    std::shared_ptr<Texture> texture = nullptr;
    std::shared_ptr<Texture> detail_texture = nullptr;
    Color diffuse_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    Color specular_color = Color(0.0f, 0.0f, 0.0f, 1.0f);
    float specular_power = 0.0f;
    bool lighting = false;
    bool lighting_specular = false;
    bool billboard = false;
    bool twosided = false;
    bool transparent = false;
    bool no_depth_write = false;

    void apply(Shader* shader) const;
};
