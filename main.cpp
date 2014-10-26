#include <stdio.h>
#include <cstddef>
#include <assert.h>
#include "main.h"

#define DEBUG false
int main ()
{
	//======== INITIAL SETUP ===========//
	int n = 0, q = 0;
	printf("Enter triangle size (n): ");
	scanf("%d", &n);
	assert(n > 0);
        
	printf("Enter random steps count (q): ");
	scanf("%d", &q); 
	assert(q > 0);

	Triangle * t = new Triangle(n);
	t->fill();
	printf("Default triangle:\n");
	t->print();

        for( int i = 0; i < q; i++ )
	{
		t->randomStep();
	}

	printf("Triangle after shuffle:\n");
	t->print();
	printf("==============================\n");

	Stack * s = new Stack;

	// first nodes inserted to stack
	for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
	{
		Direction direction = Direction(dir);
		Node * initialNode = new Node(NULL, direction, 1);
		s->push( initialNode );
	}

	Node* bestSolutionFinalNode = NULL;
	int bestCount = q;
           
        Node * lastNode = NULL;
	// ======== DEPTH-FIRST SEARCH ==========//

	while( s->getSize() > 0 )
	{
		Node* n = s->pop();
                if( DEBUG ) {
                    printf("\npopped: ");
                    t->printDirectionSymbolWin32(n->direction);
                    printf("\n");
                }
                
                while( lastNode != NULL && n->steps < lastNode->steps){
                        if( DEBUG ) printf("dead end, reverting parent\n");
                        t->move( t->oppositeDirection(lastNode->prevNode->direction) ); // revert parent move
                        lastNode = lastNode->prevNode;
                }
                lastNode = n;
                
		if( n->prevNode != NULL && n->prevNode->direction == t->oppositeDirection(n->direction) ) {// simple optimization, don't make moves there and back
                        if( DEBUG )  printf("opposite move, skipping\n");
			continue;
                }

		if( t->move(n->direction) == -1){  // INVALID_MOVE
                        if( DEBUG ) printf("invalid move, skipping\n");
			continue;
                }

		if( DEBUG ) printf("steps: %d\n", n->steps);

		if( t->isSorted() ) // this is a solution
		{
			printf("Sorted! Steps: %d; bestCount: %d\n", n->steps, bestCount);
			if( n->steps <= bestCount )
			{
				bestCount = n->steps;
				printf("New solution found with %d steps\n", bestCount);
				bestSolutionFinalNode = n;
			}
			t->move( t->oppositeDirection(n->direction) ); // revert last move
			// todo revert parent
			continue;
		}

		if( n->steps < bestCount )
		{
                        if( DEBUG ) printf("inserting moves\n");
			for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
			{
				Direction direction = Direction(dir);
				s->push( new Node(n, direction, n->steps + 1 ));
			}
		}
		else
		{
                        if( DEBUG ) printf("reverting move\n");
			t->move( t->oppositeDirection(n->direction) ); // revert last move
		}
	}

	printf("==============================\n");
	printf("End: best solution found with %d steps. Moves:\n", bestCount);
	Node* node = bestSolutionFinalNode;
	if (node == NULL)
	{
		printf("Something is wrong; no solution found.\n"); // remove after fix
		return 1;
	}
	do
	{
		t->printDirectionSymbol(node->direction);
		node = node->prevNode;
	}
	while (node != NULL);
	printf("\n");

        return 0;
}
