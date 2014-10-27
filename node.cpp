#include "node.h"
#ifndef DIRECTION_H
#define DIRECTION_H
#include "direction.h"
#endif

Node::Node(Node* prevNode, Direction direction, int steps)
{
	this->prevNode = prevNode;
	this->direction = direction;
	this->steps = steps;
}
