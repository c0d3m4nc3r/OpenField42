#include "render/shader_manager.h"

#include "utils/gl_utils.h"
#include "utils/log.h"
#include "vfs/vfs.h"

#include "glad/gl.h"

#include <sstream>

#define GLSL_VERSION "#version 450 core\n"

static GLuint buildShaderProgram(const std::string& src)
{
    const char* vertex_src[] = { GLSL_VERSION, "#define VERTEX\n", "#line 1\n", src.c_str() };
    GLuint vert_shader = GLUtils::compileShader(GL_VERTEX_SHADER, vertex_src, 4);
    if (!vert_shader) return 0;

    const char* fragment_src[] = { GLSL_VERSION, "#define FRAGMENT\n", "#line 1\n", src.c_str() };
    GLuint frag_shader = GLUtils::compileShader(GL_FRAGMENT_SHADER, fragment_src, 4);
    if (!frag_shader)
    {
        glDeleteShader(vert_shader);
        return 0;
    }

    GLuint program = GLUtils::linkProgram(vert_shader, frag_shader);

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program;
}

static bool preprocessShaderSource(std::string& src, const std::string& shader_name)
{
    std::istringstream stream(src);
    std::string line;
    int i = 0;

    while (std::getline(stream, line))
    {
        i++;
        
        if (line.find("#include") != std::string::npos)
        {
            size_t start = line.find('\"') + 1;
            size_t end = line.find('\"', start);
            std::string include_path = line.substr(start, end - start);

            std::string include_src = VFS::readFileString("shaders/" + include_path);
            if (include_src.empty())
            {
                LOG_ERROR("ShaderManager::preprocessShaderSource: Failed to read included file '%s' in shader '%s' at line %d!",
                    include_path.c_str(), shader_name.c_str(), i);
                return false;
            }

            if (!preprocessShaderSource(include_src, shader_name))
            {
                return false;
            }

            src.replace(src.find(line), line.length(), include_src);
        }
    }
    return true;
}

Shader* ShaderManager::load(
    const std::string& name,
    const std::string& path
)
{
    if (contains(name))
    {
        LOG_WARNING("ShaderManager::load: Shader '%s' already exists! Returning it...", name.c_str());
        return _shaders[name].shader.get();
    }
 
    LOG_INFO("ShaderManager::load: Loading shader '%s' from '%s'...", name.c_str(), path.c_str());

    std::string src = VFS::readFileString(path);
    if (src.empty())
    {
        LOG_ERROR("ShaderManager::load: Failed to read shader sources!");
        return nullptr;
    }

    if (!preprocessShaderSource(src, name))
    {
        LOG_ERROR("ShaderManager::load: Failed to preprocess shader '%s'!", name.c_str());
        return nullptr;
    }

    GLuint program = buildShaderProgram(src);
    if (!program) return nullptr;
    
    LOG_INFO("ShaderManager::load: Shader '%s' loaded! (ID: %u)",
        name.c_str(), program);
    
    _shaders[name] = ShaderRecord{
        .shader = std::make_unique<Shader>(program),
        .path = path
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
        std::string src = VFS::readFileString(record.path);
        if (src.empty())
        {
            LOG_ERROR("ShaderManager::reloadAll: Failed to read file for shader '%s'!", name.c_str());
            failed_count++;
            continue;
        }

        if (!preprocessShaderSource(src, name))
        {
            LOG_ERROR("ShaderManager::reloadAll: Failed to preprocess shader '%s'!", name.c_str());
            failed_count++;
            continue;
        }

        GLuint program = buildShaderProgram(src);
        if (!program)
        {
            LOG_ERROR("ShaderManager::reloadAll: Failed to build shader program for '%s'!", name.c_str());
            failed_count++;
            continue;
        }

        record.shader->setID(program);

        LOG_INFO("ShaderManager::reloadAll: Successfully reloaded '%s' (New ID: %u)", 
            name.c_str(), program);
            
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