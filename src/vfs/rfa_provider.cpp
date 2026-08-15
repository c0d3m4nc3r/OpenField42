#include "vfs/providers.h"
#include "vfs/vfs.h"
#include "utils/log.h"

using namespace VFS;

RFAProvider::RFAProvider(const std::string& archive_path)
    : _archive_path(archive_path) {}

RFAProvider::~RFAProvider()
{
    if (!_archive) return;
    RFA_Close(_archive);

    LOG_DEBUG("VFS::RFAProvider: Archive '%s' unmounted!", _archive_path.c_str());
}

bool RFAProvider::init()
{
    if (!RFA_Init())
    {
        LOG_ERROR("RFAProvider::init: Failed to initialize librfa!");
        return false;
    }

    _archive = RFA_Open(_archive_path.c_str());
    if (!_archive)
    {
        LOG_ERROR("RFAProvider::init: Failed to open archive at '%s'!", _archive_path.c_str());
        return false;
    }

    LOG_DEBUG("VFS::RFAProvider: Archive '%s' mounted!", _archive_path.c_str());

    return true;
}

bool RFAProvider::exists(const std::string& path) const
{
    if (!_archive) return false;
    return RFA_FileExists(_archive, path.c_str());
}

std::string RFAProvider::findFile(const std::string& name) const
{
    if (!_archive) return "";
    
    for (uint32_t i = 0; i < _archive->entries_count; i++)
    {
        const RFA_FileEntry* entry = &_archive->entries[i];
        if (!entry->name) continue;
        
        std::string fullpath = VFS::normalizePath(entry->name);
        
        if (fullpath.find(name) != std::string::npos)
            return fullpath;
    }

    return "";
}

std::vector<char> RFAProvider::readFile(const std::string& path)
{
    if (!_archive) return {};

    std::lock_guard<std::mutex> lock(_mutex);

    if (!exists(path))
    {
        LOG_ERROR("RFAProvider::readFile: File '%s' not found in archive!", path.c_str());
        return {};
    }

    void* data = nullptr;
    size_t size = 0;

    int result = RFA_ExtractFile(_archive, path.c_str(), &data, &size);
    if (result != 0 || !data)
    {
        LOG_ERROR("RFAProvider::readFile: Failed to extract file '%s' from archive!", path.c_str());
        return {};
    }

    std::vector<char> file_data(static_cast<uint8_t*>(data), 
                                static_cast<uint8_t*>(data) + size);

    free(data);

    return file_data;
}

std::vector<std::string> RFAProvider::listFiles()
{
    if (!_archive) return {};

    std::lock_guard<std::mutex> lock(_mutex);

    std::vector<std::string> files;
    files.resize(_archive->entries_count);

    for (uint32_t i = 0; i < _archive->entries_count; i++)
    {
        files[i] = std::string(_archive->entries[i].name);
    }

    return files;
}
