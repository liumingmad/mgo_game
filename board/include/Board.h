#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "BitArray2D.h"
#include "Node.h"
#include "json.hpp"

struct Score {
    int w_score;
    int b_score;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Score, w_score, b_score)

class Board
{
private:
    int width;
    int height;
    std::shared_ptr<Node> root;
    std::shared_ptr<Node> current;
    Score score;

public:
    int check(int x, int y, char player);
    int hasLiberty(int x, int y, char player, BitArray2D& board, std::vector<std::shared_ptr<Stone>>& markList);
    int isSuicide(int x, int y, char player, BitArray2D data);
    void scanKilled(int player, BitArray2D& data, std::vector<std::shared_ptr<Stone>>& markList);
    void scanAndRemove(int player, BitArray2D& data, std::vector<std::shared_ptr<Stone>>& markList);

    Board();
    Board(int w, int h);
    ~Board();
    int getWidth() const;
    int getHeight() const;
    int get(int x, int y);
    int move(int x, int y, char player);
    Score getScore();
    std::shared_ptr<Node> getCurrentNode() const;
    std::shared_ptr<Node> getRootNode() const;
    Score computeScore() const;    
};


void to_json(nlohmann::json& j, const Board& b);
// void from_json(const nlohmann::json& j, Board& b);


#endif // BOARD_H 