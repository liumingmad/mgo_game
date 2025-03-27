#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "BitArray2D.h"
#include "Node.h"

class Board
{
private:
    int width;
    int height;
    Node* root;
    Node* current;

public:
    int check(int x, int y, char player);
    int hasLiberty(int x, int y, char player, BitArray2D& board, std::vector<Stone>& markList);
    int isSuicide(int x, int y, char player, BitArray2D data);
    void scanKilled(int player, BitArray2D& data, std::vector<Stone>& markList);
    void scanAndRemove(int player, BitArray2D& data, std::vector<Stone>& markList);

    Board(int w, int h);
    ~Board();
    int getWidth();
    int getHeight();
    int get(int x, int y);
    int move(int x, int y, char player);
    int score(char player);
    Node& getCurrentNode();
};




#endif // BOARD_H 