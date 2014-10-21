#include <stdio.h>
#include <assert.h>
#include "triangle.h"
#include "stack.h"

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

	Stack * s = new Stack;
	Node initialNode = {NULL, RIGHT, 0};
	s->push( initialNode );

	// init best solution
	Direction * bestSteps = new Direction[q];
	int bestCount = q;

	// ======== DEPTH-FIRST SEARCH ==========//

	while( s->getSize() > 0 )
	{
		Node n = s->pop();

		if( n.prevNode != NULL && n.prevNode->lastMove == t->oppositeDirection(n.lastMove) ) // simple optimization, don't make moves there and back
			continue;

		if( t->move(n.lastMove) == -1)  // INVALID_MOVE
			continue;

		if( t->isSorted() ) // this is a solution
		{
			if( n.steps < bestCount )
			{
				bestCount = n.steps;
				printf("New solution found with %d steps->\n", bestCount);
				//TODO save best solution;
			}
			t->move( t->oppositeDirection(n.lastMove) ); // revert last move
			continue;
		}

		if( n.steps < bestCount )
		{
			for ( int dir = TOP_LEFT; dir != BOTTOM_RIGHT; dir++ ) // iterate over enum
			{
				Direction direction = Direction(dir);
				Node newNode = { &n, direction, n.steps+1 }; // where do we deallocate this shit? TODO:fix memory leak
				s->push(newNode);
			}
		}else{
			t->move( t->oppositeDirection(n.lastMove) ); // revert last move

			if( s->top() != NULL_NODE && s->top().steps < n.steps ) // dead-end
			{
				t->move( t->oppositeDirection(n.prevNode->lastMove) ); // revert parent move
			}
		}
	}
	printf("End: best solution found with %d steps", bestCount);
	scanf("%d", &q);
}
