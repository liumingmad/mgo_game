#include "Node.h"
#include "common_utils.h"

void to_json(nlohmann::json& j, std::shared_ptr<Node> node){
    j["stone"] = node->getStone();
    j["children"] = nlohmann::json::array();  // 确保是个数组

    for (auto& child : node->getChildren()) {
        nlohmann::json childJson;
        to_json(childJson, child); // 递归序列化子节点
        j["children"].push_back(childJson);
    }
}

Node::Node(int x, int y, char player, int boardWidth, int boardHeight) {
    this->stone = std::make_shared<Stone>(x, y, player);
    this->data = std::make_shared<BitArray2D>(boardWidth, boardHeight);
    this->parent = NULL;
    this->timemillis = get_now_milliseconds();
}

Node::~Node() {
}

const Stone& Node::getStone() const {
    return *(this->stone);
}

std::shared_ptr<Node> Node::getParent() {
    return this->parent;
}

void Node::setParent(std::shared_ptr<Node> node) {
    // deep copy
    std::shared_ptr<BitArray2D> p;
    if (node) {
        p = std::make_shared<BitArray2D>(*(node->data));
    } else {
        p = std::make_shared<BitArray2D>(5, 5);
    }
    int x = this->stone->x; 
    int y = this->stone->y;
    (*p)[x][y] = this->stone->color;
    this->data = p;

    // assign to parent
    this->parent = node;
}

std::vector<std::shared_ptr<Node>> Node::getChildren() const {
    return this->children;
}

void Node::addChild(std::shared_ptr<Node> node) {
    this->children.push_back(node);
}

void Node::removeChild(std::shared_ptr<Node> node) {
    for (int i = 0; i < this->children.size(); i++) {
        if (this->children[i]->getStone().x == node->getStone().x &&
            this->children[i]->getStone().y == node->getStone().y &&
            this->children[i]->getStone().color == node->getStone().color) {
            this->children.erase(this->children.begin() + i);
            return;
        }
    }
}