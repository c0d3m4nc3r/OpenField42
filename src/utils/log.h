#pragma once

#define LOG_DEBUG(format, ...) \
    logMsg(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    logMsg(LOG_LEVEL_INFO, format, ##__VA_ARGS__)

#define LOG_WARNING(format, ...) \
    logMsg(LOG_LEVEL_WARNING, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    logMsg(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

typedef enum : unsigned char {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
} LogLevel;

void logMsg(LogLevel level, const char* format, ...);