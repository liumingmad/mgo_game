#ifndef UTILS_H
#define UTILS_H

#include "BitArray2D.h"

void print_board(const BitArray2D& board);

void print_board_p(BitArray2D* board);

void log(std::string msg);

char get_opponent(char player);

#endif // UTILS_H