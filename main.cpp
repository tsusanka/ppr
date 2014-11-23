#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <assert.h>
#include "mpi.h"
#include "main.h"

#define DEBUG false
#define CHECK_MSG_AMOUNT  100

#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN        1003
#define MSG_FINISH       1004

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
	int tag = 1;
	int flag;
	MPI_Status status;

	/* start up MPI */
	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcessors);

	/* Sequential Variables */
	int bestCount;
	Triangle * t;

	if( my_rank == 0 )
	{
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
		const char * message = t->convertToString().c_str();
		for (int destination = 1; destination < numberOfProcessors; )
		{
			MPI_Send( (void*) message, strlen(message)+1, MPI_CHAR, destination, tag, MPI_COMM_WORLD );
			destination++;
		}

		bestCount = q;
	}
	else
	{
		MPI_Status status;
		int flag = 0;
		//WAIT FOR SHUFFLED TRIANGLE
		while (!flag)
		{
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status); //busy waiting(?)
		}
		char message[1000]; // todo: what length?
		MPI_Recv( &message, 1000, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
		printf ("%s\n",message);

		// parse message
		t = 0; // PARSE TRIANGLE
		bestCount = 8;// PARSE NUMBER OF SHUFFLES;
	}
	MPI_Finalize();
	return 0;
	Stack * s = new Stack;

	// first nodes inserted to stack
	for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
	{
		Direction direction = Direction(dir);
		Node * initialNode = new Node(NULL, direction, 1);
		s->push( initialNode );
	}

	int toInitialSend = my_rank == 0 ? numberOfProcessors-1 : 0;
	Direction * bestSolution = new Direction[bestCount];

	Node * lastNode = new Node(NULL, RIGHT, 1);

	int checkMsgCounter = 0;

	// ======== DEPTH-FIRST SEARCH ==========//

	while( s->getSize() > 0 )
	{   
                if ((checkMsgCounter % CHECK_MSG_AMOUNT)==0)
                {
                        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
                        if (flag)
                        {
                                //prisla zprava, je treba ji obslouzit
                                //v promenne status je tag (status.MPI_TAG), cislo odesilatele (status.MPI_SOURCE)
                                //a pripadne cislo chyby (status.MPI_ERROR)
                                switch (status.MPI_TAG)
                                {
                                   case MSG_WORK_REQUEST : // zadost o praci, prijmout a dopovedet
                                                           // zaslat rozdeleny zasobnik a nebo odmitnuti MSG_WORK_NOWORK
                                                           break;
                                   case MSG_WORK_SENT : // prisel rozdeleny zasobnik, prijmout
                                                        // deserializovat a spustit vypocet
                                                        break;
                                   case MSG_WORK_NOWORK : // odmitnuti zadosti o praci
                                                          // zkusit jiny proces
                                                          // a nebo se prepnout do pasivniho stavu a cekat na token
                                                          break;
                                   case MSG_TOKEN : //ukoncovaci token, prijmout a nasledne preposlat
                                                    // - bily nebo cerny v zavislosti na stavu procesu
                                                    break;
                                   case MSG_FINISH : //konec vypoctu - proces 0 pomoci tokenu zjistil, ze jiz nikdo nema praci
                                                     //a rozeslal zpravu ukoncujici vypocet
                                                     //mam-li reseni, odeslu procesu 0
                                                     //nasledne ukoncim spoji cinnost
                                                     //jestlize se meri cas, nezapomen zavolat koncovou barieru MPI_Barrier (MPI_COMM_WORLD)
                                                     MPI_Finalize();
                                                     exit (0);
                                                     break;
                                   default : printf("neznamy typ zpravy!\n"); break;
                              }
                        }
                }
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
