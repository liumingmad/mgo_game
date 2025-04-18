// #include "timer.h"
// #include <global.h>
// #include "common_utils.h"

// bool check_move_timeout(Room& room) {
//     const Board& board = room.board;
//     long now = get_now_milliseconds();
//     long move_time = board.getCurrentNode().getTimemillis();
//     long duration = now - move_time;

//     bool is_empty_board = board.getRootNode().getChildren().empty();
//     if (is_empty_board) {
//         // 第一步不能超过30s
//         if (duration < 30*1000) {
//             return false;
//         }

//         // 1. push message timeout
//         // 2. room -> game over

//     } else {
//         // 其他步不能超过，约定的时间限制
//         if (room.remain_pretime > 0) {
//             return false;
//         }

//         if (room.remain_movetime > 0) {
//             return false;
//         }

//         room.remain_read_second_count--;
//         room.remain_movetime = room.moveTime;
//     }

//     return false;
// }

// // 检查所有room，服务器主动发起的推送
// void onTrigger() {
//     // std::cout << "GameTimerCallback onTrigger()" << std::endl;

//     if (g_rooms.size() == 0) return;

//     for (const auto& [key, value] : g_rooms) {
//         Room room = value;

//         // 1.检查落子是否超时
//         check_move_timeout(room);

//         // 2.对手离线
//         // 当对手离线时，在player对象里保存了上线和离线的时间戳列表

//         // 3.对手上线
//     }

// }