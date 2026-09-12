#include "utils/string_utils.h"

#include <algorithm>
#include <charconv>
#include <cctype>

namespace StringUtils
{
    std::vector<std::string_view> split(std::string_view s, char delimiter)
    {
        std::vector<std::string_view> tokens;
        size_t start = 0;
        size_t end = s.find(delimiter);

        while (end != std::string_view::npos)
        {
            std::string_view token = s.substr(start, end - start);
            if (!token.empty())
                tokens.push_back(token);
            
            start = end + 1;
            end = s.find(delimiter, start);
        }

        std::string_view last_token = s.substr(start);
        if (!last_token.empty())
            tokens.push_back(last_token);

        return tokens;
    }

    std::string lowercase(std::string_view s)
    {
        std::string result(s);
        std::transform(result.begin(), result.end(), result.begin(), 
                    [](unsigned char c){ return std::tolower(c); });
        return result;
    }

    std::string toString(const glm::vec2& val)
    {
        return std::to_string(val.x) + '/' + std::to_string(val.y);
    }

    std::string toString(const glm::vec3& val)
    {
        return std::to_string(val.x) + '/' + std::to_string(val.y) + '/' + std::to_string(val.z);
    }
    
    template <>
    int fromString<int>(std::string_view str)
    {
        int val = 0;
        if (!str.empty()) {
            std::from_chars(str.data(), str.data() + str.size(), val);
        }
        return val;
    }

    template <>
    float fromString<float>(std::string_view str)
    {
        float val = 0.0f;
        if (!str.empty()) {
            std::from_chars(str.data(), str.data() + str.size(), val);
        }
        return val;
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
    glm::vec2 fromString<glm::vec2>(std::string_view str)
    {
        const auto parts = split(str, '/');
        if (parts.size() >= 2) {
            return glm::vec2(
                fromString<float>(parts[0]),
                fromString<float>(parts[1])
            );
        } else if (parts.size() == 1) {
            return glm::vec2(
                fromString<float>(parts[0])
            );
        }
        return glm::vec2(0.0f);
    }

    template <>
    glm::vec3 fromString<glm::vec3>(std::string_view str)
    {
        const auto parts = split(str, '/');
        if (parts.size() >= 3) {
            return glm::vec3(
                fromString<float>(parts[0]),
                fromString<float>(parts[1]),
                fromString<float>(parts[2])
            );
        } else if (parts.size() == 1) {
            return glm::vec3(
                fromString<float>(parts[0])
            );
        }
        return glm::vec3(0.0f);
    }

    bool hasNoDigits(std::string_view str)
    {
        return std::none_of(str.begin(), str.end(), [](unsigned char c) {
            return std::isdigit(c);
        });
    }
}