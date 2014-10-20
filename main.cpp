#include <stdio.h>
#include <assert.h>
#include "triangle.h"

#define INIT_STACK_SIZE 500

struct Node {
    Node prevNode;
    Direction lastMove;
    int steps;
};


//======== STACK (LIFO) ===============//
int stackCapacity;
Node * stack;
int stackSize;
int maxSteps;

void push( Node n )
{
    stack[ stackSize++ ] = n;  //TODO check capacity and inflate the stack if necessary
}


Node pop( )
{
    if( stackSize == 0 ) return NULL;
    return stack[ --stackSize ];
}

//=====================================


main ()
{
        //======== INITIAL SETUP ===========//
	int n = 0, q = 0;
	printf("Enter triangle size (n):");
	scanf("%d", &n);
	assert(n > 0);
	printf("Enter random steps count (q):");
	scanf("%d", &q);
	assert(q > 0);

	Triangle * t = new Triangle(n);
	t->fill();
	t->print();
        
        for( int i = 0; i < q; i++ ){
            t->randomStep();
        }
        t->print();
        
        
        // ======== DEPTH-FIRST SEARCH ==========//
        
        // init stack
        stackCapacity = INIT_STACK_SIZE;
        stack = new Node*[stackCapacity];
        stackSize = 0;
        maxSteps = q;
        //push first node
        Node initialNode = {NULL,0,0};
        push( initialNode );
        
        
        while( stackSize > 0 ){
            Node n = pop();
            t->move(n.lastMove);
            if( n.steps < maxSteps ){
                
                for ( int dir = TOP_LEFT; dir != BOTTOM_RIGHT; dir++ ) // iterate over enum
                {
                   Direction direction = Direction(dir);
                   Node newNode = { n, direction, n.steps+1 };
                   push(newNode);
                }
            }
            
        }
                
        scanf("%d", &q);
}

