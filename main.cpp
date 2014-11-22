#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <assert.h>
#include "main.h"

#define DEBUG false
//#define WINDOWS false
#define LENGTH 100

void copySolution( Direction * where, Node * from )
{
	Node* node = from;
	int i = 0;
	if (node == NULL)
	{
		printf("Something is wrong; no solution found.\n"); // remove after fix
		return;
	}

	do
	{
		where[i++] = node->direction;
		node = node->prevNode;
	}
	while (node != NULL);
}

int main( int argc, char** argv )
{
    
        /* MPI VARIABLES */
        int my_rank;
        
        int numberOfProcessors;
        
        char message[LENGTH];
        /* start up MPI */
        MPI_Init( &argc, &argv );

        /* find out process rank */
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

        /* find out number of processes */
        MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcessors);
  
        
        /* Sequential Variables */
        int bestCount;
        Triangle * t;
        
        if( my_rank == 0 ){
                //INIT ARGS AND TRIANGLE AND SEND TO OTHER PROCESSES
                if (argc != 3)
                {
                        printf("not enough or too much arguments\n");
                        return 1;
                }

                int n = 0, q = 0;
                if (sscanf (argv[1], "%i", &n) != 1)
                {
                        printf("error - not an integer\n");
                        return 2;
                }
                assert(n > 0);

                if (sscanf (argv[2], "%i", &q) != 1)
                {
                        printf("error - not an integer\n");
                        return 3;
                }
                assert(q > 0);

                t = new Triangle(n);
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
                
                // SEND
                for (int source=1;source<p;) {
                    MPI_Send( message, strlen(message)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD );
                }
                
                bestCount = q;
                
        }else{
                MPI_Status status;
                int flag = 0;
                //WAIT FOR SHUFFLED TRIANGLE
                while (!flag){
                    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status); //busy waiting(?)
                }
                MPI_Recv( &message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
                printf ("%s\n",message);
                
                // parse message
                t = 0// PARSE TRIANGLE
                bestCount = 8;// PARSE NUMBER OF SHUFFLES;
        }
	Stack * s = new Stack;

	// first nodes inserted to stack
	for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
	{
		Direction direction = Direction(dir);
		Node * initialNode = new Node(NULL, direction, 1);
		s->push( initialNode );
	}
        
        
	int toInitialSend = myRank == 0 ? numberOfProcessors-1 : 0;
	Direction * bestSolution = new Direction[bestCount];

	Node * lastNode = new Node(NULL, RIGHT, 1);
	// ======== DEPTH-FIRST SEARCH ==========//

	while( s->getSize() > 0 )
	{
		Node* n = s->pop();

		if( DEBUG )
		{
			printf("\npopped: ");
			t->printDirectionSymbol(n->direction);
			printf("\n");
		}

		while( lastNode->prevNode != NULL && n->steps < lastNode->steps)
		{
			if( DEBUG ) printf("dead end, reverting parent\n");
			t->move( t->oppositeDirection(lastNode->prevNode->direction) ); // revert parent move

			Node * lastNodePrevNode = lastNode->prevNode;
			lastNode->direction = lastNodePrevNode->direction;   // lastNode = lastNodePrevNode
			lastNode->steps = lastNodePrevNode->steps;
			lastNode->prevNode = lastNodePrevNode->prevNode;
			delete lastNodePrevNode;
		}


		if( n->prevNode != NULL && n->prevNode->direction == t->oppositeDirection(n->direction) ) // simple optimization, don't make moves there and back
		{
			if( DEBUG ) printf("opposite move, skipping\n");
			delete n;
			continue;
		}

		if( t->move(n->direction) == -1) // INVALID_MOVE
		{
			if( DEBUG ) printf("invalid move, skipping\n");
			delete n;
			continue;
		}

		lastNode->direction = n->direction;  // lastNode = lastNode->prevNode
		lastNode->prevNode = n->prevNode;
		lastNode->steps = n->steps;

		if( DEBUG ) printf("steps: %d\n", n->steps);

		if( t->isSorted() ) // this is a solution
		{
			printf("Sorted! Steps: %d; bestCount: %d\n", n->steps, bestCount);
			if( n->steps <= bestCount )
			{
				bestCount = n->steps;
				printf("New solution found with %d steps\n", bestCount);
				copySolution( bestSolution, n);
			}
			t->move( t->oppositeDirection(n->direction) ); // revert last move
			// todo revert parent
			continue;
		}

		if( n->steps < bestCount )
		{
                        if( toInitialSend > 0 ){
                            //SEND THIS NODE TO PROCESSOR P
                            toInitialSend--;
                        }else{
                            if( DEBUG ) printf("inserting moves\n");
                            for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
                            {
                                    Direction direction = Direction(dir);
                                    s->push( new Node(n, direction, n->steps + 1 ));
                            }
                        }
		}
		else
		{
			if( DEBUG ) printf("reverting move\n");
			t->move( t->oppositeDirection(n->direction) ); // revert last move
			delete n;
		}
	}

	printf("==============================\n");
	printf("End: best solution found with %d steps. Moves:\n", bestCount);
	for( int i = bestCount - 1; i >= 0; i-- )
	{
		t->printDirectionSymbol(bestSolution[i]);
	}
	delete [] bestSolution;
	printf("\n");

	delete s;
	delete t;
	delete lastNode;

	return 0;
}
