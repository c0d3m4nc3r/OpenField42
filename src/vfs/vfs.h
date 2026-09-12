#pragma once

#include <memory>

class IFileProvider;
class VFS
{
public:

    bool mountProvider(const std::shared_ptr<IFileProvider>& provider);
    void unmountAll();

    bool exists(std::string_view path);
    std::string findFile(std::string_view name);
    std::vector<char> readFile(std::string_view path);
    std::string readFileString(std::string_view path);
    std::vector<std::string> listFiles(std::string_view path);
    std::string normalizePath(std::string_view path);

private:

    std::vector<std::shared_ptr<IFileProvider>> _providers;
};
