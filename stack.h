#ifndef DIRECTION_H
#define DIRECTION_H
#include "direction.h"
#endif

#ifndef NULL
#define NULL 0
#endif

typedef struct NodeT
{
	struct NodeT* prevNode;
	Direction lastMove;
	int steps;

	bool operator==(const NodeT& rhs) const
	{
		return  rhs.steps == this->steps &&
				rhs.prevNode == this->prevNode &&
				rhs.lastMove == this->lastMove;
	}

	bool operator!=(const NodeT& rhs) const
	{
		return  !(this == &rhs);
	}

} Node;

const Node NULL_NODE = {NULL, RIGHT, 0};

class Stack
{

	protected:
		int capacity;
		int size;
		Node * nodes;

	public:
		Stack();
		~Stack();
		void push(Node n);
		Node pop();
		const Node top();
		int getSize();
};
