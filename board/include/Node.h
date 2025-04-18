#ifndef NODE_H
#define NODE_H

#include <cstddef>
#include <vector>
#include "Stone.h"
#include "BitArray2D.h"

class Node
{
    private:
        // 落子的时间戳
        long timemillis;
        Stone* stone;
        Node* parent;
        std::vector<Node> children;

    public:
        BitArray2D* data;
        Node(int x, int y, char player, int w, int h);
        ~Node();
        Stone& getStone();
        Node* getParent();
        void setParent(Node* node);
        std::vector<Node> getChildren();
        void addChild(Node* node);
        void removeChild(Node* node);
        long getTimemillis();
};


#endif // NODE_H