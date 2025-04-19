#include "Node.h"
#include "common_utils.h"

using namespace std;

Node::Node(int x, int y, char player, int boardWidth, int boardHeight) {
    this->stone = new Stone(x, y, player);
    this->data = new BitArray2D(boardWidth, boardHeight);
    this->parent = NULL;
    this->timemillis = get_now_milliseconds();
}

Node::~Node() {
    delete this->stone;
    delete this->data;
}

Stone& Node::getStone() {
    return *(this->stone);
}

Node* Node::getParent() {
    return this->parent;
}

void Node::setParent(Node* node) {
    // deep copy
    BitArray2D* p = NULL;
    if (node) {
        p = new BitArray2D(*(node->data));
    } else {
        p = new BitArray2D(5, 5);
    }
    int x = this->stone->x; 
    int y = this->stone->y;
    (*p)[x][y] = this->stone->color;
    this->data = p;

    // assign to parent
    this->parent = node;
}

vector<Node> Node::getChildren() {
    return this->children;
}

void Node::addChild(Node* node) {
    this->children.push_back(*node);
}

void Node::removeChild(Node* node) {
    for (int i = 0; i < this->children.size(); i++) {
        if (this->children[i].getStone().x == node->getStone().x &&
            this->children[i].getStone().y == node->getStone().y &&
            this->children[i].getStone().color == node->getStone().color) {
            this->children.erase(this->children.begin() + i);
            return;
        }
    }
}