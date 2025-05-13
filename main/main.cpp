#include <iostream>
#include "Server.h"
#include "wrap.h"
#include "Log.h"
#include "ServiceGuard.h"

// #include <signal.h>
// void signal_handler(int sig) {
//     std::abort();
// }

void cleanup() {
    SPDLOG_INFO("Stopping worker threads...");
    // 模拟清理
    spdlog::shutdown(); // 刷盘
    SPDLOG_INFO("Resources cleaned up.");
}

int main(int argc, char const *argv[])
{
    init_async_logger(true);

    ServiceGuard::get_instance().start(cleanup);

    Server server;
    server.init();
    server.run(9001);
    return 0;
}

