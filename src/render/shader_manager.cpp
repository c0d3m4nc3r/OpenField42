#include "render/shader_manager.h"

#include "utils/gl_utils.h"
#include "utils/log.h"
#include "vfs/vfs.h"

#include "glad/gl.h"

Shader* ShaderManager::load(
    const std::string& name,
    const std::string& vert_path,
    const std::string& frag_path
)
{
    if (contains(name))
    {
        LOG_WARNING("ShaderManager::load: Shader '%s' already exists! Returning it...", name.c_str());
        return _shaders[name].shader.get();
    }
 
    LOG_INFO("ShaderManager::load: Loading shader '%s' from '%s' and '%s'...",
        name.c_str(), vert_path.c_str(), frag_path.c_str());

    std::string vert_src = VFS::readFileString(vert_path);
    std::string frag_src = VFS::readFileString(frag_path);

    if (vert_src.empty() || frag_src.empty())
    {
        LOG_ERROR("ShaderManager::load: Failed to read shader sources!");
        return nullptr;
    }

    GLuint vert_shader = GLUtils::compileShader(vert_src.c_str(), GL_VERTEX_SHADER);
    if (!vert_shader) return nullptr;

    GLuint frag_shader = GLUtils::compileShader(frag_src.c_str(), GL_FRAGMENT_SHADER);
    if (!frag_shader)
    {
        glDeleteShader(vert_shader);
        return nullptr;
    }

    unsigned int program = GLUtils::linkProgram(vert_shader, frag_shader);
    
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    if (!program) return nullptr;
    
    LOG_INFO("ShaderManager::load: Shader '%s' loaded! (ID: %u)",
        name.c_str(), program);
    
    _shaders[name] = ShaderRecord{
        .shader = std::make_unique<Shader>(program),
        .vert_path = vert_path,
        .frag_path = frag_path
    };

    return _shaders[name].shader.get();
}

void ShaderManager::unload(const std::string& name)
{
    auto it = _shaders.find(name);
    if (it != _shaders.end())
    {
        _shaders.erase(it);
        LOG_INFO("ShaderManager::unload: Shader '%s' unloaded!", name.c_str());
    }
    else
    {
        LOG_ERROR("ShaderManager::unload: Shader '%s' not found!", name.c_str());
    }
}

void ShaderManager::unloadAll()
{
    _shaders.clear();

    LOG_INFO("ShaderManager::unloadAll: Unloaded all shaders!");
}

void ShaderManager::reloadAll()
{
    LOG_INFO("ShaderManager::reloadAll: Reloading all shaders...");

    size_t reloaded_count = 0;
    size_t failed_count = 0;

    for (auto& [name, record] : _shaders)
    {
        std::string vert_src = VFS::readFileString(record.vert_path);
        std::string frag_src = VFS::readFileString(record.frag_path);

        if (vert_src.empty() || frag_src.empty())
        {
            LOG_ERROR("ShaderManager::reloadAll: Failed to read files for shader '%s'!", name.c_str());
            failed_count++;
            continue;
        }

        GLuint vertex_shader = GLUtils::compileShader(vert_src.c_str(), GL_VERTEX_SHADER);
        if (!vertex_shader)
        {
            LOG_ERROR("ShaderManager::reloadAll: Failed to compile vertex shader for '%s'!", name.c_str());
            failed_count++;
            continue;
        }

        GLuint fragment_shader = GLUtils::compileShader(frag_src.c_str(), GL_FRAGMENT_SHADER);
        if (!fragment_shader)
        {
            LOG_ERROR("ShaderManager::reloadAll: Failed to compile fragment shader for '%s'!", name.c_str());
            glDeleteShader(vertex_shader);
            failed_count++;
            continue;
        }

        GLuint new_program_id = GLUtils::linkProgram(vertex_shader, fragment_shader);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        if (!new_program_id)
        {
            LOG_ERROR("ShaderManager::reloadAll: Failed to link shader program for '%s'!", name.c_str());
            failed_count++;
            continue;
        }

        record.shader->setID(new_program_id);

        LOG_INFO("ShaderManager::reloadAll: Successfully reloaded '%s' (New ID: %u)", 
            name.c_str(), new_program_id);
            
        reloaded_count++;
    }

    LOG_INFO("ShaderManager::reloadAll: Done. Reloaded: %zu, Failed: %zu", 
        reloaded_count, failed_count);
}

Shader* ShaderManager::get(const std::string& name) const
{
    auto it = _shaders.find(name);
    return it != _shaders.end() ? it->second.shader.get() : nullptr;
}

bool ShaderManager::contains(const std::string& name) const
{
    return _shaders.contains(name);
}