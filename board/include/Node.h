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
        std::shared_ptr<Stone> stone;
        std::shared_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> children;

    public:
        std::shared_ptr<BitArray2D> data;
        Node(int x, int y, char player, int w, int h);
        ~Node();
        const Stone& getStone() const;
        std::shared_ptr<Node> getParent();
        void setParent(std::shared_ptr<Node> node);
        std::vector<std::shared_ptr<Node>> getChildren() const;
        void addChild(std::shared_ptr<Node> node);
        void removeChild(std::shared_ptr<Node> node);
        long getTimemillis();
};
void to_json(nlohmann::json& j, std::shared_ptr<Node> node);
// void from_json(const nlohmann::json& j, Board& b);

#endif // NODE_H