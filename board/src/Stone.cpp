#include "Stone.h"

Stone::Stone(int x, int y, char player) {
    this->x = x;
    this->y = y;
    this->player = player;
}

Stone::~Stone() {
}

int Stone::getX() {
    return this->x;
}

int Stone::getY() {
    return this->y;
}

char Stone::getPlayer() {
    return this->player;
}
