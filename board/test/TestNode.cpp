#include <iostream>

#include "utils.h"
#include "../include/Node.h"

void print_node(Node* node) {
    std::cout << "-------------------------" << std::endl;
    std::cout << "Node: " 
        << node->getStone().getX() << ", " << node->getStone().getY() << ", " << node->getStone().getPlayer()
        << std::endl;
    print_board(*(node->data));
    std::cout << std::endl;
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
