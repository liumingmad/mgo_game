#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include "BitArray2D.h"
#include <memory>
void print_board(const BitArray2D& board)
{
    int rows = board.getRows();
    int cols = board.getCols();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            std::cout << board[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "------------------------" << std::endl;
}

void print_board_p(std::shared_ptr<BitArray2D> board) {
    print_board(*board);
}

void log(std::string msg) {
    std::cout << msg << std::endl;
}

char get_opponent(char player) {
    if (player == 'B') return 'W';
    return 'B';
}

#endif // UTILS_H