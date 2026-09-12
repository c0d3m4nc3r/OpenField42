#pragma once

#include "vfs/providers.h"

#include "librfa/rfa.h"

#include <mutex>
#include <vector>
#include <string>

class IFileProvider
{
public:

    virtual ~IFileProvider() = default;
    virtual bool init() = 0;
    virtual bool exists(std::string_view path) const = 0;
    virtual std::string findFile(std::string_view name) const = 0;
    virtual std::vector<char> readFile(std::string_view path) = 0;
    virtual std::vector<std::string> listFiles() = 0;
};

class FolderProvider : public IFileProvider
{
public:

    FolderProvider(const std::string& archive_path);
    ~FolderProvider();
    
    bool init() override;
    bool exists(std::string_view path) const override;
    std::string findFile(std::string_view name) const override;
    std::vector<char> readFile(std::string_view path) override;
    std::vector<std::string> listFiles() override;

private:

    std::string _base_path;
};

class RFAProvider : public IFileProvider
{
public:

    RFAProvider(const std::string& archive_path);
    ~RFAProvider();
    
    bool init() override;
    bool exists(std::string_view path) const override;
    std::string findFile(std::string_view name) const override;
    std::vector<char> readFile(std::string_view path) override;
    std::vector<std::string> listFiles() override;

private:

    RFA_Archive* _archive;
    std::string _archive_path;
    mutable std::mutex _mutex;
};
