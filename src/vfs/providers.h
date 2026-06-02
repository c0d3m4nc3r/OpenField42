#pragma once

#include <vector>
#include <string>

#include "librfa/rfa.h"

class IFileProvider
{
public:

    virtual ~IFileProvider() = default;
    virtual bool init() = 0;
    virtual bool exists(const std::string& path) const = 0;
    virtual std::string findFile(const std::string& name) const = 0;
    virtual std::vector<char> readFile(const std::string& path) = 0;
    virtual std::vector<std::string> listFiles() = 0;
};

class FolderProvider : public IFileProvider
{
public:

    FolderProvider(const std::string& path);
    bool init() override;
    bool exists(const std::string& path) const override;
    std::string findFile(const std::string& name) const override;
    std::vector<char> readFile(const std::string& path) override;
    std::vector<std::string> listFiles() override;
    
private:

    std::string base_path;
};

class RFAProvider : public IFileProvider
{
public:

    RFAProvider(const std::string& archive_path);
    ~RFAProvider();
    
    bool init() override;
    bool exists(const std::string& path) const override;
    std::string findFile(const std::string& name) const override;
    std::vector<char> readFile(const std::string& path) override;
    std::vector<std::string> listFiles() override;

private:

    RFA_Archive* archive;
    std::string archive_path;
};
