#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "BitArray2D.h"
#include "Node.h"
#include "json.hpp"

class Board
{
private:
    int width;
    int height;
    std::shared_ptr<Node> root;
    std::shared_ptr<Node> current;

public:
    int check(int x, int y, char player);
    int hasLiberty(int x, int y, char player, BitArray2D& board, std::vector<Stone>& markList);
    int isSuicide(int x, int y, char player, BitArray2D data);
    void scanKilled(int player, BitArray2D& data, std::vector<Stone>& markList);
    void scanAndRemove(int player, BitArray2D& data, std::vector<Stone>& markList);

    Board();
    Board(int w, int h);
    ~Board();
    int getWidth() const;
    int getHeight() const;
    int get(int x, int y);
    int move(int x, int y, char player);
    int score(char player);
    std::shared_ptr<Node> getCurrentNode() const;
    std::shared_ptr<Node> getRootNode() const;
};


void to_json(nlohmann::json& j, const Board& b);
// void from_json(const nlohmann::json& j, Board& b);


#endif // BOARD_H 