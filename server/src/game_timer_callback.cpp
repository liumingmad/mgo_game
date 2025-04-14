#include "timer.h"
#include <global.h>

void GameTimerCallback::onTrigger() {
    // std::cout << "GameTimerCallback onTrigger()" << std::endl;

    // 检查所有room，服务器主动发起的推送

    if (g_rooms.size() == 0) return;

    // 1.检查落子是否超时
    // 棋谱树的每个节点，带上落子的时间戳

    // 2.对手离线
    // 当对手离线时，在player对象里保存了上线和离线的时间戳列表

    // 3.对手上线
}