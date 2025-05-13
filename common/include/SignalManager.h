#pragma once
#include <csignal>
#include <map>
#include <functional>
#include <spdlog/spdlog.h>

class SignalManager {
public:
    using Handler = std::function<void(int)>;

    static void register_handler(int signal, Handler handler) {
        get_instance().handlers[signal] = std::move(handler);

        struct sigaction sa {};
        sa.sa_handler = dispatch;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;

        if (sigaction(signal, &sa, nullptr) == -1) {
            SPDLOG_ERROR("Failed to register handler for signal {}", signal);
        } else {
            SPDLOG_INFO("Registered signal handler for {}", signal);
        }
    }

    static void ignore_signal(int signal) {
        if (signal == SIGKILL || signal == SIGSTOP) {
            SPDLOG_WARN("Cannot ignore signal {}", signal);
        } else {
            ::signal(signal, SIG_IGN);
            SPDLOG_INFO("Signal {} set to ignore", signal);
        }
    }

    static void restore_default(int signal) {
        ::signal(signal, SIG_DFL);
        SPDLOG_INFO("Signal {} set to default handler", signal);
    }

private:
    static void dispatch(int signal) {
        auto& handlers = get_instance().handlers;
        auto it = handlers.find(signal);
        if (it != handlers.end()) {
            SPDLOG_WARN("Caught signal {}, dispatching to handler...", signal);
            it->second(signal);
        } else {
            SPDLOG_WARN("Caught signal {}, but no handler registered", signal);
        }
    }

    static SignalManager& get_instance() {
        static SignalManager instance;
        return instance;
    }

    std::map<int, Handler> handlers;
};