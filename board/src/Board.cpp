#include <iostream>
#include <string>
#include "Board.h"
#include "../include/utils.h"
#include "Log.h"

using namespace std;

void to_json(nlohmann::json& j, const Board& b) {
    j = nlohmann::json{
        {"width", b.getWidth()},
        {"height", b.getHeight()},
        // {"data", },
    };
    Node& curr = b.getCurrentNode();
    j["data"] = std::string(curr.data->getData());
}

// void from_json(const nlohmann::json& j, Board& b) {
//     b.setWidth(j.at("width").get<int>());
//     b.setHeight(j.at("height").get<int>());
//     if (j.contains("root")) {
//         auto root = new Node();
//         j.at("root").get_to(*root);
//         b.setRoot(root);
//     }
// }

Node& Board::getRootNode() const {
    return *root;
}

Board::Board() : Board(19, 19) {
}

Board::Board(int w, int h) {
    this->width = w;
    this->height = h;
    this->root = new Node(-1, -1, 'T', w, h);
    this->current = this->root;
}

Board::~Board() {
}

int Board::getWidth() const {
    return this->width;
}

int Board::getHeight() const {
    return this->height;
}

int Board::get(int x, int y) {
    return *(this->current->data)[x][y];
}

int Board::move(int x, int y, char player) {
    if (check(x, y, player) == 0) return 0;
    if (isSuicide(x, y, player, *(this->current->data))) {
        Log::error("Error: move() is suicide.");
        return 0;
    }

    // add node to tree
    Node* node = new Node(x, y, player, this->width, this->height);
    node->setParent(this->current);
    this->current->addChild(node);
    this->current = node;

    // remove captured stones
    std::vector<Stone> killedList;
    scanAndRemove(get_opponent(player), *(this->current->data), killedList);

    return 1;
}

// 扫描全盘，获取被拿掉的棋子
void Board::scanKilled(int player, BitArray2D& data, std::vector<Stone>& markList) {
    for (int x=0; x<this->width; x++) {
        for (int y=0; y<this->height; y++) {
            if (data[x][y] != player) continue;
            std::vector<Stone> list; 
            if (hasLiberty(x, y, player, data, list) == 0) {
                markList.insert(markList.end(), list.begin(), list.end());
            }
        }
    }
}

// 扫描棋盘，移除无气的棋子
void Board::scanAndRemove(int player, BitArray2D& data, std::vector<Stone>& markList) {
    scanKilled(player, data, markList);

    // remove
    for (Stone one : markList) {
        int x = one.x;
        int y = one.y;
        data[x][y] = '0';
    }

    // 'X'->'player'
    for (int x=0; x<this->width; x++) {
        for (int y=0; y<this->height; y++) {
            if (data[x][y] == 'X') {
                data[x][y] = player;
            }
        }
    }
}

Node & Board::getCurrentNode() const
{
    return *(this->current);
}

int Board::check(int x, int y, char player) {
    if (x < 0 || x >= this->width) return 0;
    if (y < 0 || y >= this->height) return 0;
    if (player != 'B' && player != 'W') return 0;
    if (this->current != NULL) {
        BitArray2D& data = *(this->current->data);
        if (data[x][y] != '0') return 0;
    }
    return 1;
}

// 是否是自杀
int Board::isSuicide(int x, int y, char player, BitArray2D data) {
    data[x][y] = player;

    // 1. 如果有气，不算自杀
    BitArray2D tmp = data;
    std::vector<Stone> list;
    if (hasLiberty(x, y, player, tmp, list)) {
        log("isSuicide() hasLiberty > 0");
        return 0;
    }

    // 2. 如果没气，但是提吃了对方的棋子，不算自杀
    BitArray2D copy = data;
    std::vector<Stone> killedList;
    scanAndRemove(get_opponent(player), copy, killedList);
    if (killedList.size() > 1) {
        log("isSuicide() killedList.size()");
        return 0;
    }  

    // 3. 如果被提吃的子是1个，则有可能是打劫，不算自杀
    // 如果当前节点的孩子节点与当前节点的父节点相同，则认为盘面重复
    if (killedList.size() == 1) {
        Node* pNode = this->current->getParent();
        if (pNode == NULL) {
            log("isSuicide() parent is NULL");
            return 0;
        }
        // 盘面重复
        if (*(pNode->data) != copy) {
            log("isSuicide() is not ko");
            return 0;
        } 
    }

    return 1;
}

// 当这个位置落子后，是否有气
int Board::hasLiberty(int x, int y, char player, BitArray2D& tmp, std::vector<Stone>& markList) {
    if (tmp[x][y] == 'X') return 0;
    if (x < 0 || x >= this->width) return 0;
    if (y < 0 || y >= this->height) return 0;
    if (tmp[x][y] == '0') return 1;
    if (tmp[x][y] == get_opponent(player)) return 0;

    // 标记为已检查
    tmp[x][y] = 'X';
    markList.push_back(*(new Stone(x, y, player)));

    // 递归检查上下左右
    int hasLiberty = 0;
    hasLiberty += this->hasLiberty(x-1, y, player, tmp, markList);
    hasLiberty += this->hasLiberty(x+1, y, player, tmp, markList);
    hasLiberty += this->hasLiberty(x, y-1, player, tmp, markList);
    hasLiberty += this->hasLiberty(x, y+1, player, tmp, markList);
    return hasLiberty;
}

int Board::score(char player) {
    return 0;
}

