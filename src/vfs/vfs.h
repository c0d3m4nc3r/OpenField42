#pragma once

#include "vfs/providers.h"

#include <memory>

namespace VFS
{
    static std::vector<std::shared_ptr<IFileProvider>> providers;

    bool mountProvider(std::shared_ptr<IFileProvider> provider);
    bool exists(const std::string& path);
    std::string findFile(const std::string& name);
    std::vector<char> readFileData(const std::string& path);
    std::string readFileString(const std::string& path);
    std::vector<std::string> listFiles(const std::string& path);
    std::string normalizePath(const std::string& path);
}
