#include "core/engine.h"
#include "core/config.h"
#include "utils/log.h"

#include <SDL3/SDL_timer.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>

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

bool hasNoDigits(const std::string& name)
{
    return std::none_of(name.begin(), name.end(), [](unsigned char c) {
        return std::isdigit(c);
    });
}

void listLevels()
{
    LOG_INFO("Levels list:");
    fs::path levels_dir = std::string(GAME_DATA_DIR) + "/bf1942/Archives/bf1942/levels";

    int i = 0;

    for (const auto& entry : fs::directory_iterator(levels_dir))
    {
        if (entry.is_regular_file())
        {
            std::string filename = entry.path().filename().string();

            if (hasNoDigits(filename))
            {
                if (filename.length() > 4)
                    filename = filename.substr(0, filename.length() - 4);

                i++;
                LOG_INFO("%d: %s", i, filename.c_str());
            }
        }
    }
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

    for (int i = 0; i < argc; i++)
    {
        if (strncmp(argv[i], "-l", 2) == 0)
        {
            listLevels();
            return 0;
        }
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
