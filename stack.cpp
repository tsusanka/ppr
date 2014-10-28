#include "stack.h"
#include <stdio.h>

#define INIT_STACK_SIZE 15

#ifndef NULL
#define NULL 0;
#endif

Stack::Stack( )
{
	capacity = INIT_STACK_SIZE;
	nodes = new Node*[capacity];
	size = 0;
}

void Stack::destroy( )
{
	printf("deleting capacity: %d \n", capacity);
	for (int i = 0; i < capacity; i++)
	{
		delete nodes[i];
	}
	delete [] nodes;
}

Stack::~Stack( )
{
	destroy();
}

void Stack::push( Node* n )
{
	if( size >= capacity - 1 ) //stack is full
	{
		expand();
	}
	nodes[ size++ ] = n;
}

void Stack::expand()
{
	int newSize = capacity * 1.5;
	printf("Stack: new capacity: %d\n", newSize);
	// realloc
	Node** temp = new Node*[newSize];
	for( int i = 0; i < capacity; i++)
	{
		temp[i] = nodes[i];
	}
	destroy();
	nodes = temp;
	capacity = newSize;
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
