#ifndef DIRECTION_H
#define DIRECTION_H
#include "direction.h"
#endif

class Node
{

	public:
		Node(Node* prevNode, Direction direction, int steps);
		Node* prevNode;
		Direction direction;
		int steps;

};
