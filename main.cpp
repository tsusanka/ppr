#include <stdio.h>
#include <assert.h>
#include "triangle.h"

#define INIT_STACK_SIZE 500

typedef struct NodeT {
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
} Node; //c++ sucks

const Node NULL_NODE = {NULL,RIGHT,0};

//======== STACK (LIFO) ===============//
int stackCapacity;
Node * stack;
int stackSize;

void push( Node n )
{
    stack[ stackSize++ ] = n;  //TODO check capacity and inflate the stack if necessary
}


Node pop( )
{
    if( stackSize == 0 ) 
        return NULL_NODE;
    return stack[ --stackSize ];
}


const Node top( )
{
    if( stackSize == 0 ) 
        return NULL_NODE;
    return stack[ stackSize - 1  ];
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
        stack = new Node[stackCapacity];
        stackSize = 0;

        //init best solution
        Direction * bestSolution = new Direction[q];
        int bestSolutionSteps = q;
        
        //push first node
        Node initialNode = { NULL, RIGHT, 0};
        push( initialNode );        
        
        while( stackSize > 0 ){
            Node n = pop();

            if( n.prevNode != NULL && n.prevNode->lastMove == t->oppositeDirection(n.lastMove) ) //simple optimization, don't make moves there and back
                continue;
            
            if( t->move(n.lastMove) == -1) 
                continue;   //INVALID_MOVE
            
            if( t->isSorted() ){ //this is a solution
                if( n.steps < bestSolutionSteps ){
                    bestSolutionSteps = n.steps;
                    printf("New solution found with %d steps", bestSolutionSteps);
                    //TODO save best solution;
                }
                t->move( t->oppositeDirection(n.lastMove) ); //revert last move
                continue;
            }
            
            if( n.steps < bestSolutionSteps ){
                for ( int dir = TOP_LEFT; dir != BOTTOM_RIGHT; dir++ ) // iterate over enum
                {
                   Direction direction = Direction(dir);
                   Node newNode = { &n, direction, n.steps+1 }; //where do we deallocate this shit? TODO:fix memory leak
                   push(newNode);
                }
            }else{
                t->move( t->oppositeDirection(n.lastMove) ); //revert last move
                
                if( top() != NULL_NODE && top().steps < n.steps ){ // dead-end
                    t->move( t->oppositeDirection(n.prevNode->lastMove) ); //revert parent move
                }
            }
        }
        printf("End: best solution found with %d steps", bestSolutionSteps);       
        scanf("%d", &q);
}

