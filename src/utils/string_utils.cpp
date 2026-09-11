#include "utils/string_utils.h"

#include <sstream>
#include <algorithm>

#include <cctype>

namespace StringUtils
{
    std::vector<std::string> split(const std::string& s, char delimiter)
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(s);

        while (std::getline(iss, token, delimiter))
        {
            if (!token.empty())
                tokens.push_back(token);
        }

        return tokens;
    }

    std::string lowercase(const std::string& s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), 
                    [](unsigned char c){ return std::tolower(c); });
        return result;
    }

    std::string toString(const glm::vec2& val)
    {
        return std::to_string(val.x) + "/" + std::to_string(val.y);
    }

    std::string toString(const glm::vec3& val)
    {
        return std::to_string(val.x) + "/" + std::to_string(val.y) + "/" + std::to_string(val.z);
    }

    template <>
    int fromString<int>(std::string_view str)
    {
        if (str.empty()) return 0;
        try {
            return std::stoi(std::string(str));
        } catch (...) {
            return 0;
        }
    }

    template <>
    float fromString<float>(std::string_view str)
    {
        if (str.empty()) return 0.0f;
        try {
            return std::stof(std::string(str));
        } catch (...) {
            return 0.0f;
        }
    }

    template <>
    bool fromString<bool>(std::string_view str)
    {
        return str == "1" || str == "true" || str == "TRUE";
    }

    template <>
    std::string fromString<std::string>(std::string_view str)
    {
        return std::string(str);
    }

    template <>
    glm::vec2 fromString<glm::vec2>(std::string_view str) {
        const auto parts = split(std::string(str), '/');
        if (parts.size() < 2) {
            return glm::vec2(0.0f);
        }
        return glm::vec2(
            fromString<float>(parts[0]),
            fromString<float>(parts[1])
        );
    }

    template <>
    glm::vec3 fromString<glm::vec3>(std::string_view str) {
        const auto parts = split(std::string(str), '/');
        if (parts.size() < 3) {
            return glm::vec3(0.0f);
        }
        return glm::vec3(
            fromString<float>(parts[0]),
            fromString<float>(parts[1]),
            fromString<float>(parts[2])
        );
    }
}