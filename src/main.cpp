#include "core/engine.h"
#include "core/config.h"
#include "utils/log.h"

#include <SDL3/SDL_timer.h>
#include <iostream>
#include <string>

#include <filesystem>

namespace fs = std::filesystem;

bool checkRequiredDirs(const std::vector<std::string>& required_dirs)
{
    bool all_found = true;
    
    try
    {
        const fs::path current_path = fs::current_path();
        LOG_INFO("Starting resource check. Current working directory: %s", current_path.string().c_str());
    }
    catch (const fs::filesystem_error& e)
    {
        LOG_ERROR("Could not determine current path: %s", e.what());
        return false;
    }

    for (const auto& dir_name : required_dirs)
    {
        fs::path dir_path = fs::current_path() / dir_name;
        
        LOG_DEBUG("Checking for path: %s", dir_path.string().c_str());

        if (fs::exists(dir_path))
        {
            if (fs::is_directory(dir_path))
            {
                LOG_INFO("Required directory found: %s", dir_name.c_str());
            }
            else
            {
                LOG_WARNING("Path exists but is not a directory: %s", dir_name.c_str());
                all_found = false;
            }
        }
        else
        {
            LOG_ERROR("CRITICAL RESOURCE MISSING: Directory not found: %s", dir_name.c_str());
            all_found = false;
        }
    }
    
    return all_found;
}

int main(int argc, char* argv[])
{
    if (!checkRequiredDirs({GAME_DATA_DIR, ASSETS_DIR}))
    {
        LOG_ERROR("Program initialization failed! Critical resources are missing!");
        std::cout << "Press ENTER to exit...\n";
        std::cin.get();
        return 1;
    }

    Engine engine;

    if (!engine.init(argc, argv))
        return 2;
    
    Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 previous_counter = SDL_GetPerformanceCounter();
    
    while (engine.isRunning())
    {
        Uint64 current_counter = SDL_GetPerformanceCounter();
        float delta_time = (float)(current_counter - previous_counter) / frequency;
        previous_counter = current_counter;

        float fps = 0.0f;
        if (delta_time > 0.0f)
            fps = 1.0f / delta_time;

        engine.tick(delta_time, fps, frequency);
    }

    engine.shutdown();
    
    return 0;
}
