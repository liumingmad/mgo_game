
#include <chrono>
#include <random>
#include <iostream>
#include <execinfo.h>

long get_now_milliseconds() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

int gen_random(int min, int max) {
    std::random_device rd;                          // 硬件随机数种子
    std::mt19937 gen(rd());                         // 梅森旋转算法随机数引擎
    std::uniform_int_distribution<> dis(min, max);     // 定义范围 0~99 的均匀分布
    return dis(gen);
}

void print_stacktrace() {
    const int max_frames = 100;
    void* addrlist[max_frames];

    int addrlen = backtrace(addrlist, max_frames);
    if (addrlen == 0) {
        std::cerr << "  <empty stack trace>\n";
        return;
    }

    char** symbol_list = backtrace_symbols(addrlist, addrlen);
    for (int i = 0; i < addrlen; i++) {
        std::cerr << symbol_list[i] << "\n";
    }
    free(symbol_list);
}