#ifndef DIRECTION_H
#define DIRECTION_H
#include "direction.h"
#endif

class Node
{
	// todo capsulate

	public:
		Node(Node* prevNode, Direction direction, int steps);
		~Node();
		void incrementTries();
		Node* prevNode;
		Direction direction;
		int steps;
		int tries;

};
