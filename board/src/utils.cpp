#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include "BitArray2D.h"
#include <memory>
#include "Log.h"
void print_board(const BitArray2D& board)
{
    int rows = board.getRows();
    int cols = board.getCols();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            LOG_INFO(board[i][j] + " ");
        }
        LOG_INFO("\n");
    }
    LOG_INFO("------------------------\n" );
}

void print_board_p(std::shared_ptr<BitArray2D> board) {
    print_board(*board);
}

void log(std::string msg) {
    LOG_INFO(msg);
}

char get_opponent(char player) {
    if (player == 'B') return 'W';
    return 'B';
}

#endif // UTILS_H