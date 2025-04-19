#include <iostream>

#include "../include/utils.h"
#include "../include/Board.h"

void testHasLiberty() {
    Board* board = new Board(5, 5);
    BitArray2D tmp(5, 5);
    tmp[0][1] = 'B';
    tmp[1][0] = 'B';
    tmp[1][2] = 'B';
    tmp[2][2] = 'B';
    tmp[3][0] = 'B';
    tmp[3][1] = 'B';
    
    tmp[1][1] = 'W';
    tmp[2][0] = 'W';
    tmp[2][1] = 'W';
    print_board(tmp);

    std::vector<Stone> list;
    int has = board->hasLiberty(1, 1, 'W', tmp, list);
    print_board(tmp);
    std::cout << "hasLiberty=" << has << std::endl;
    for (Stone one : list) {
        std::cout << one.x << "," << one.y << std::endl;
    }
}

void testScanAndRemove() {
    Board* board = new Board(5, 5);
    BitArray2D tmp(5, 5);
    tmp[0][1] = 'B';
    tmp[1][0] = 'B';
    tmp[1][2] = 'B';
    tmp[2][2] = 'B';
    tmp[3][0] = 'B';
    tmp[3][1] = 'B';
    
    tmp[1][1] = 'W';
    tmp[2][0] = 'W';
    tmp[2][1] = 'W';
    print_board(tmp);

    std::vector<Stone> list;
    board->scanAndRemove('W', tmp, list);
    for (Stone one : list) {
        std::cout << one.x << "," << one.y << std::endl;
    }
    print_board(tmp);
}

void testIsSuicide1() {
    Board* board = new Board(5, 5);
    BitArray2D tmp(5, 5);
    tmp[0][1] = 'B';
    tmp[1][0] = 'B';
    tmp[1][2] = 'B';
    tmp[2][2] = 'B';
    tmp[3][0] = 'B';
    tmp[3][1] = 'B';
    
    tmp[1][1] = 'W';
    // tmp[2][0] = 'W';
    tmp[2][1] = 'W';
    print_board(tmp);

    std::vector<Stone> list;
    int suicide = board->isSuicide(2, 0, 'W', tmp);
    print_board(tmp);
    std::cout << "suicide=" << suicide << std::endl;
}

void testIsSuicide2() {
    Board* board = new Board(5, 5);
    BitArray2D tmp(5, 5);
    tmp[0][0] = 'B';
    tmp[1][0] = 'B';
    tmp[2][1] = 'B';
    tmp[3][0] = 'B';
    
    tmp[0][1] = 'W';
    tmp[1][1] = 'W';
    print_board(tmp);

    std::vector<Stone> list;
    int suicide = board->isSuicide(2, 0, 'W', tmp);
    print_board(tmp);
    std::cout << "suicide=" << suicide << std::endl;
}

void testMove() {
    Board* board = new Board(5, 5);
    board->move(1, 0, 'B');
    board->move(2, 1, 'B');
    board->move(3, 0, 'B');
    
    board->move(0, 0, 'W');
    board->move(0, 1, 'W');
    board->move(1, 1, 'W');
    print_board_p(board->getCurrentNode().data);

    // 提掉一个子
    board->move(2, 0, 'W');
    print_board_p(board->getCurrentNode().data);

    // suicide
    board->move(1, 0, 'B');
    print_board_p(board->getCurrentNode().data);
    
    // 找劫财
    board->move(4, 0, 'B');
    print_board_p(board->getCurrentNode().data);

    // 应劫财
    board->move(4, 1, 'W');
    print_board_p(board->getCurrentNode().data);

    // 再提劫
    board->move(1, 0, 'B');
    print_board_p(board->getCurrentNode().data);
}

int main(int argc, char const *argv[])
{
    // testHasLiberty();
    // testScanAndRemove();
    testMove();
    return 0;
}
