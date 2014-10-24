#include "stack.h"
#include <cstddef>

#define INIT_STACK_SIZE 500

Stack::Stack( )
{
	capacity = INIT_STACK_SIZE;
	nodes = new Node*[capacity];
	size = 0;
}

Stack::~Stack( )
{
	// TODO destroy
}

void Stack::push( Node* n )
{
	nodes[ size++ ] = n;  //TODO check capacity and inflate the stack if necessary
}

Node* Stack::pop( )
{
	if( size == 0 )
		return NULL;
	return nodes[ --size ];
}

const Node* Stack::top( )
{
	if( size == 0 )
		return NULL;
	return nodes[ size - 1  ];
}

int Stack::getSize( )
{
	return size;
}
