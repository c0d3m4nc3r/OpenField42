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
}