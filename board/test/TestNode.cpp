#include <iostream>

#include "utils.h"
#include "../include/Node.h"
#include <Log.h>

void print_node(Node* node) {
    LOG_INFO("-------------------------\n");
    Stone s = node->getStone();
    LOG_INFO("Node: {}, {}, {}", s.x, s.y, s.color);
    print_board(*(node->data));
    LOG_INFO("\n");
}

int main(int argc, char const *argv[])
{
    Node* root = new Node(0, 1, 'B', 5, 5);
    root->setParent(NULL);

    Node* child1 = new Node(1, 0, 'B', 5, 5);
    child1->setParent(root);
    root->addChild(child1);
    print_node(child1);

    Node* child2 = new Node(3, 3, 'B', 5, 5);
    child2->setParent(child1);
    child1->addChild(child2);

    print_node(child2);
    return 0;
}
