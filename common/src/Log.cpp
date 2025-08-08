#include "Log.h"

#include <spdlog/spdlog.h>
#include <spdlog/async.h>  // 必须包含这个
#include <spdlog/sinks/basic_file_sink.h>

// 在 Docker / Kubernetes 中的使用
// •	日志输出到 stdout：
// 让容器平台（如 FluentBit）负责采集日志
// auto stdout_logger = spdlog::stdout_color_mt("console");
// spdlog::set_default_logger(stdout_logger);

// spdlog::set_level(spdlog::level::off); // 静默测试日志

/*
void init_async_logger() {
    spdlog::init_thread_pool(8192, 1);

    auto async_logger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>(
        "async_logger",
        "logs/async.log",
        1048576 * 5, // 5 MB
        3            // 保留3个文件
    );

    spdlog::set_default_logger(async_logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][%t] %v");
}
*/

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

void init_async_logger(bool to_console) {
    // 初始化异步线程池：队列大小 8192，后台线程数 1
    spdlog::init_thread_pool(8192, 1);

    // 创建 sink 列表
    std::vector<spdlog::sink_ptr> sinks;

    // 文件 sink（使用容器内的日志目录）
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "/app/logs/async.log", true);
    sinks.push_back(file_sink);

    if (to_console) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks.push_back(console_sink);
    }

    // 创建异步 logger（多 sink）
    auto async_logger = std::make_shared<spdlog::async_logger>(
        "async_logger",
        sinks.begin(),
        sinks.end(),
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block
    );

    async_logger->set_level(spdlog::level::info);
    async_logger->flush_on(spdlog::level::warn);
    spdlog::set_default_logger(async_logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%l][%t] %v");
}