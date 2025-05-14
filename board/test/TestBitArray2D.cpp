#include <iostream>

#include "utils.h"
#include "../include/Board.h"
#include "Log.h"

int main(int argc, char const *argv[])
{
    const BitArray2D board(5, 5);
    board[0][3] = 'X';
    board[2][1] = 'B';
    board[2][2] = 'W';
    board[4][1] = 'W';
    print_board(board);

    const BitArray2D board2(board);
    board2[4][4] = 'B';
    print_board(board2);

    const BitArray2D board3 = board;
    print_board(board3);

    LOG_INFO(board == board2);
    LOG_INFO(board3 == board);
    return 0;
}