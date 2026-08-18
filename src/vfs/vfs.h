#pragma once

#include <memory>

class IFileProvider;

class VFS
{
public:

    bool mountProvider(const std::shared_ptr<IFileProvider>& provider);
    void unmountAll();

    bool exists(const std::string& path);
    std::string findFile(const std::string& name);
    std::vector<char> readFileData(const std::string& path);
    std::string readFileString(const std::string& path);
    std::vector<std::string> listFiles(const std::string& path);
    std::string normalizePath(const std::string& path);

private:

    std::vector<std::shared_ptr<IFileProvider>> _providers;
};
