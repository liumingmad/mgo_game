#ifndef LOG_H
#define LOG_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>

#define LOG_INFO(...)    SPDLOG_INFO(__VA_ARGS__)
#define LOG_DEBUG(...)   SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_ERROR(...)   SPDLOG_ERROR(__VA_ARGS__)

void init_async_logger(bool to_console);

#endif // LOG_H