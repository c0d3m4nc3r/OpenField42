#include "utils/log.h"

#include <cstdio>
#include <cstdarg>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

static inline void enableAnsiOnWindows()
{
#ifdef _WIN32
    static bool enabled = false;
    if (enabled) return;

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);

    enabled = true;
#endif
}

void logMsg(LogLevel level, const char* format, ...)
{
#ifdef _WIN32
    enableAnsiOnWindows();
#endif

    // ----- Time -----
    time_t now = time(nullptr);
    tm* tm_info = localtime(&now);

    char timestamp[10];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm_info);

    // ----- Level -----
    const char* level_str = "UNKNOWN";
    const char* level_color = "\033[0m";

    switch (level)
    {
    case LOG_LEVEL_DEBUG:   level_str = "DEBUG";   level_color = "\033[34m"; break;
    case LOG_LEVEL_INFO:    level_str = "INFO";    level_color = "\033[32m"; break;
    case LOG_LEVEL_WARNING: level_str = "WARNING"; level_color = "\033[33m"; break;
    case LOG_LEVEL_ERROR:   level_str = "ERROR";   level_color = "\033[31m"; break;
    }

    // ----- Print -----
    va_list args;
    va_start(args, format);

    fprintf(stderr, "%s%s | [%s] > ", level_color, timestamp, level_str);
    vfprintf(stderr, format, args);

    fprintf(stderr, "\033[0m\n"); // reset color

    va_end(args);
}
