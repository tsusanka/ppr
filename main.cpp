#include <stdio.h>
#include <cstddef>
#include <assert.h>
#include "main.h"

#define TOTAL_DIRECTION_COUNT 6

main ()
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
	for ( int dir = TOP_LEFT; dir != BOTTOM_RIGHT; dir++ )
	{
		Direction direction = Direction(dir);
		Node * initialNode = new Node(NULL, direction, 1);
		s->push( initialNode );
	}

	Node* bestSolutionFinalNode = NULL;
	int bestCount = q;

	// ======== DEPTH-FIRST SEARCH ==========//

	Node* node = NULL;
	while( s->getSize() > 0 )
	{
		if( node && node->prevNode != NULL && node->prevNode->tries == TOTAL_DIRECTION_COUNT ) // all leafs are gone; time to revert the parent
		{
			t->move( t->oppositeDirection(node->prevNode->direction) ); // revert parent move
		}

		node = s->pop();
		node->incrementTries();
		if (node->prevNode)
			printf("doting %d\n", node->prevNode->tries);

		// if( node->prevNode != NULL && node->prevNode->direction == t->oppositeDirection(node->direction) ) // simple optimization, don't make moves there and back
		// {
			// node->incrementTries();
			// continue;
		// }

		if( t->move(node->direction) == -1) // INVALID_MOVE
			continue;

		printf("steps: %d\n", node->steps);

		if( t->isSorted() ) // this is a solution
		{
			printf("Sorted! Steps: %d; bestCount: %d\n", node->steps, bestCount);
			if( node->steps <= bestCount )
			{
				bestCount = node->steps;
				printf("New solution found with %d steps\n", bestCount);
				bestSolutionFinalNode = node;
			}
			t->move( t->oppositeDirection(node->direction) ); // revert last move
			// node->incrementTries();
			// todo revert one more time?
			// foreach kamosi ktery maji stejny steps move opposite direction
			continue;
		}

		if( node->steps < bestCount )
		{
			for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
			{
				Direction direction = Direction(dir);
				s->push( new Node(node, direction, node->steps + 1 ));
			}
		}
		else
		{
			t->move( t->oppositeDirection(node->direction) ); // revert last move
		}
	}

	printf("==============================\n");
	printf("End: best solution found with %d steps. Moves:\n", bestCount);
	Node* finalNode = bestSolutionFinalNode;
	if (finalNode == NULL)
	{
		printf("Something is wrong; no solution found.\n"); // TODO remove after fix
		return 1;
	}
	do
	{
		t->printDirectionSymbol(finalNode->direction); // TODO print in right order; non-reverse
		finalNode = finalNode->prevNode;
	}
	while (finalNode != NULL);
	printf("\n");
}
