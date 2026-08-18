#include "vfs/vfs.h"
#include "utils/log.h"
#include "utils/string_utils.h"
#include "vfs/providers.h"

#include <algorithm>
    
bool VFS::mountProvider(const std::shared_ptr<IFileProvider>& provider)
{
    if (!provider || !provider->init())
        return false;
    
    _providers.insert(_providers.begin(), provider); 

    return true;
}

void VFS::unmountAll()
{
    LOG_INFO("VFS::unmountAll: Unmounting all _providers...");

    _providers.clear();

    LOG_INFO("VFS::unmountAll: All _providers unmounted successfully!");
}

bool VFS::exists(const std::string& path)
{
    std::string normalized_path = normalizePath(path);

    for(auto& p : _providers)
        if(p->exists(normalized_path)) return true;

    return false;
}

std::string VFS::findFile(const std::string& name)
{
    std::string normalized_name = normalizePath(name);

    for(auto& p : _providers)
    {
        std::string path = p->findFile(normalized_name);
        if (!path.empty())
            return path;
    }

    return "";
}

std::vector<char> VFS::readFile(const std::string& path)
{
    std::string full_path = findFile(path); 

    if (full_path.empty())
    {
        LOG_ERROR("VFS::readFile: File '%s' not found!", path.c_str());
        return {};
    }

    for(auto& p : _providers)
    {
        if (p->exists(full_path)) 
            return p->readFile(full_path);
    }
        
    LOG_ERROR("VFS::readFile: File '%s' found at '%s' but could not be read!",
        path.c_str(), full_path.c_str());
    return {};
}

std::string VFS::readFileString(const std::string& path)
{
    std::string full_path = findFile(path); 

    if (full_path.empty())
    {
        LOG_ERROR("VFS::readFileData: File '%s' not found!", path.c_str());
        return {};
    }

    for(auto& p : _providers)
    {
        if(p->exists(full_path))
        {
            std::vector<char> data = p->readFile(full_path);
            return std::string(data.begin(), data.end()); 
        }
    }
    
    LOG_ERROR("VFS::readFileString: File '%s' found at '%s' but could not be read!",
        path.c_str(), full_path.c_str());
    return {};
}

std::vector<std::string> VFS::listFiles(const std::string& path)
{
    std::vector<std::string> files;

    for (auto& provider : _providers)
    {
        auto provider_files = provider->listFiles();
        for (const auto& f : provider_files)
        {
            if (f.starts_with(path))
                files.push_back(f);
        }
    }

    return files;
}

std::string VFS::normalizePath(const std::string& path)
{
    std::string result = StringUtils::lowercase(path);
    
    std::replace(result.begin(), result.end(), '\\', '/');
    
    std::string::iterator new_end = std::unique(result.begin(), result.end(),
        [](char a, char b) { return a == '/' && b == '/'; });
    result.erase(new_end, result.end());
    
    if (result.length() > 1)
    {
        if (result.back() == '/') result.pop_back();
        if (result.front() == '/') result = result.substr(1);
    }
    
    return result;
}