#pragma once

#include <vector>
#include <string>

namespace StringUtils
{
    std::vector<std::string_view> split(std::string_view s, char delimiter = ' ');
    std::string lowercase(std::string_view s);

    inline std::string toString(const std::string& val) { return val; }
    inline std::string toString(std::string_view val)   { return std::string(val); }
    inline std::string toString(bool val)               { return val ? "true" : "false"; }
    inline std::string toString(int val)                { return std::to_string(val); }
    inline std::string toString(float val)              { return std::to_string(val); }

    std::string toString(const glm::vec2& val);
    std::string toString(const glm::vec3& val);

    template <typename T>
    T fromString(std::string_view str);

    template <> int fromString<int>(std::string_view str);
    template <> float fromString<float>(std::string_view str);
    template <> bool fromString<bool>(std::string_view str);
    template <> std::string fromString<std::string>(std::string_view str);
    template <> glm::vec2 fromString<glm::vec2>(std::string_view str); // x/y
    template <> glm::vec3 fromString<glm::vec3>(std::string_view str); // x/y/z
}