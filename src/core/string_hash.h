#pragma once

struct StringHash
{
    using is_transparent = void;

    size_t operator()(std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }
    size_t operator()(const std::string& str) const {
        return std::hash<std::string>{}(str);
    }
};