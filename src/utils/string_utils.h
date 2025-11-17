#pragma once

#include <vector>
#include <string>

namespace StringUtils
{
    std::vector<std::string> split(const std::string& s, char delimiter = ' ');
    std::string lowercase(const std::string& s);
}