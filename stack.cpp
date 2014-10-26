#include "stack.h"
#include <stdio.h>


#define INIT_STACK_SIZE 15
#define NULL 0;
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
        if( size >= capacity - 1 ) //stack is full
        {
            expand();
        }
        nodes[ size++ ] = n;
}

void Stack::expand() 
{
    int oldSize = capacity;
    capacity *= 1.5;
    printf("Stack: new capacity: %d\n",capacity);
    // realloc
    Node** temp = new Node*[capacity];
    for( int i = 0; i < oldSize; i++){
        temp[i] = nodes[i];
    }
    delete [] nodes;
    nodes = temp;
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
