#pragma once

#include "render/shader.h"

class ShaderManager
{
public:

    Shader* load(
        const std::string& name,
        const std::string& vert_path,
        const std::string& frag_path
    );

    void unload(const std::string& name);
    void unloadAll();
    
    void reloadAll();

    Shader* get(const std::string& name) const;

    bool contains(const std::string& name) const;

private:

    struct ShaderRecord
    {
        std::unique_ptr<Shader> shader;
        std::string vert_path, frag_path;
    };

    std::unordered_map<std::string, ShaderRecord> _shaders;
};
