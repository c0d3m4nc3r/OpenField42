#include "vfs/providers.h"
#include "vfs/vfs.h"
#include "core/log.h"

RFAProvider::RFAProvider(const std::string& archive_path)
    : archive_path(archive_path) {}

RFAProvider::~RFAProvider()
{
    if (!archive) return;
    RFA_Close(archive);
}

bool RFAProvider::init()
{
    if (!RFA_Init())
    {
        LOG_ERROR("RFAProvider::init: Failed to initialize librfa!");
        return false;
    }

    archive = RFA_Open(archive_path.c_str());
    if (!archive)
    {
        LOG_ERROR("RFAProvider::init: Failed to open archive at '%s'!", archive_path.c_str());
        return false;
    }

    return true;
}

bool RFAProvider::exists(const std::string& path) const
{
    if (!archive) return false;
    return RFA_FileExists(archive, path.c_str());
}

std::string RFAProvider::findFile(const std::string& name) const
{
    if (!archive) return "";
    
    for (uint32_t i = 0; i < archive->entries_count; i++)
    {
        const RFA_FileEntry* entry = &archive->entries[i];
        if (!entry->name) continue;
        
        std::string fullpath = VFS::normalizePath(entry->name);
        
        if (fullpath.find(name) != std::string::npos)
            return fullpath;
    }

    return "";
}

std::vector<char> RFAProvider::readFile(const std::string& path)
{
    if (!archive) return {};

    if (!exists(path))
    {
        LOG_ERROR("RFAProvider::readFile: File '%s' not found in archive!", path.c_str());
        return {};
    }

    void* data = nullptr;
    size_t size = 0;

    int result = RFA_ExtractFile(archive, path.c_str(), &data, &size);
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
    if (!archive) return {};

    std::vector<std::string> files;
    files.resize(archive->entries_count);

    for (uint32_t i = 0; i < archive->entries_count; i++)
    {
        files[i] = std::string(archive->entries[i].name);
    }

    return files;
}
