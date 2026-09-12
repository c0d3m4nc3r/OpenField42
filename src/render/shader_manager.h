#pragma once

#include "core/string_hash.h"
#include "render/shader.h"

class ShaderManager
{
public:

    Shader* load(std::string_view name, std::string_view path);
    void unload(std::string_view name);
    void unloadAll();
    
    void reloadAll();

    Shader* get(std::string_view name) const;

    bool contains(std::string_view name) const;

private:

    struct ShaderRecord
    {
        std::unique_ptr<Shader> shader;
        std::string path;
    };

    std::unordered_map<std::string, ShaderRecord, StringHash, std::equal_to<>> _shaders;
};
