#pragma once
#include "SignalManager.h"
#include <atomic>
#include <functional>
#include <spdlog/spdlog.h>

class ServiceGuard {
public:
    using CleanupFunc = std::function<void()>;

    ServiceGuard() = default;

    void start(CleanupFunc cleanup_fn) {
        cleanup_function = std::move(cleanup_fn);
        running.store(true);

        // 注册系统信号
        SignalManager::register_handler(SIGINT,  [](int) { get_instance().on_exit_signal("SIGINT"); });
        SignalManager::register_handler(SIGTERM, [](int) { get_instance().on_exit_signal("SIGTERM"); });
        SignalManager::register_handler(SIGSEGV, [](int) { get_instance().on_exit_signal("SIGSEGV"); });
        SignalManager::register_handler(SIGABRT, [](int) { get_instance().on_exit_signal("SIGABRT"); });
        SignalManager::ignore_signal(SIGPIPE);

        SPDLOG_INFO("ServiceGuard started");
    }

    static ServiceGuard& get_instance() {
        static ServiceGuard instance;
        return instance;
    }

    static bool is_running() {
        return get_instance().running.load();
    }

private:
    void on_exit_signal(const std::string& signal_name) {
        if (!running.exchange(false)) return;

        SPDLOG_WARN("Received {}, triggering shutdown...", signal_name);
        if (cleanup_function) {
            cleanup_function();
        } else {
            SPDLOG_ERROR("No cleanup function registered.");
        }

        SPDLOG_INFO("Shutdown complete.");
        exit(0);
    }

    std::atomic<bool> running{false};
    CleanupFunc cleanup_function;
};