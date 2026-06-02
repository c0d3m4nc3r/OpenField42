#include "vfs/providers.h"
#include "utils/log.h"

#include <filesystem>
#include <fstream>

FolderProvider::FolderProvider(const std::string& path)
    : _base_path(path) {}

bool FolderProvider::init()
{
    if (!std::filesystem::exists(_base_path) || !std::filesystem::is_directory(_base_path))
    {
        LOG_ERROR("FolderProvider::init: Path '%s' doesn't exist or is not a directory!", _base_path.c_str());
        return false;
    }

    return true;
}

bool FolderProvider::exists(const std::string& path) const
{
    std::filesystem::path full_path = std::filesystem::path(_base_path) / path;
    return std::filesystem::exists(full_path) && std::filesystem::is_regular_file(full_path);
}

std::string FolderProvider::findFile(const std::string& name) const
{
    std::filesystem::path target_path(name);
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(_base_path))
    {
        if (entry.is_regular_file())
        {
            std::filesystem::path relative_path = std::filesystem::relative(entry.path(), _base_path);
            
            if (relative_path == target_path)
            {
                return relative_path.string();
            }
        }
    }
    
    return "";
}

std::vector<char> FolderProvider::readFile(const std::string& path)
{
    std::filesystem::path full_path = std::filesystem::path(_base_path) / path;
    std::ifstream file(full_path, std::ios::binary | std::ios::ate);
    
    if (!file.is_open())
    {
        LOG_ERROR("FolderProvider::readFile: Failed to open file '%s'", full_path.string().c_str());
        return {};
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        LOG_ERROR("FolderProvider::readFile: Failed to read file '%s'", full_path.string().c_str());
        return {};
    }

    return buffer;
}

std::vector<std::string> FolderProvider::listFiles()
{
    std::vector<std::string> files;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(_base_path))
    {
        if (entry.is_regular_file())
        {
            std::string relative_path = std::filesystem::relative(entry.path(), _base_path).string();
            files.push_back(relative_path);
        }
    }

    return files;
}