#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <assert.h>
#include "mpi.h"
#include "main.h"
#include <unistd.h>
#include <time.h>

#define DEBUG false
#define CHECK_MSG_AMOUNT  100
#define LENGTH 1000

#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN        1003
#define MSG_FINISH       1004

#define WORK       8004
#define IDLE       8005
#define FINISH       8006
#define TOKEN       8007

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

Direction * getPath(Node * lastNode)
{
    Direction * result = new Direction[LENGTH];
    int i = 0;
	do
	{
		result[i++] = lastNode->direction;
		lastNode = lastNode->prevNode;
	}
	while (lastNode != NULL);
    result[i] = NONE;
    return result;
}

void fillStackFromMessage( Stack * s, Triangle * t, char * message )
{
    int position = 0;
	int number;
    Direction direction;
    Node * lastNode = NULL;
    int i = 1;
    while(true)
    {
        MPI_Unpack(message, LENGTH, &position, &number, 1, MPI_INT, MPI_COMM_WORLD);
        direction = (Direction) number;
        if( direction == NONE )
        {
            break;
        }
        int result = t->move ( direction );
        if( result == -1 )
        {
            printf("fillStackFromMessage>Invalid MOVE recieved:%d",number);
        }
        Node * n = new Node(lastNode, direction, i++);
        lastNode = n;
    }
    t->move( t->oppositeDirection(lastNode->direction) ); //revert the last move, because it will be done after it is popped from the stack
    s->push(lastNode);
}

/**
 * Sends node to defined processor
 */
void sendWork(int to, Node * lastNode )
{
    char * buffer = new char[LENGTH];
    int position = 0;
    Direction * result = getPath(lastNode);
    int ri = 0;
    do
    {
        int a = (int) result[ri++];
        MPI_Pack(&a, 1, MPI_INT, buffer, LENGTH, &position, MPI_COMM_WORLD);
    }
    while( result[ri] != NONE);
    MPI_Send( (void*) buffer, position, MPI_PACKED, to, MSG_WORK_SENT, MPI_COMM_WORLD );
}

/**
 * Deals with all the work
 */
int workState( Stack * s, int toInitialSend, Triangle * t, int myRank, int bestCount )
{
    int checkMsgCounter = 0;
    int flag;
    char message[LENGTH];
    MPI_Status status;
    Direction * bestSolution = new Direction[bestCount];
    int sendWorkTo = -1;

    // the first work ever that is sent to other processors
    if( myRank != 0)
    {
        while (true)
        {
            MPI_Iprobe(0, MSG_WORK_SENT, MPI_COMM_WORLD, &flag, &status);
            if( flag ) break;
        }
        MPI_Recv( message, LENGTH, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
        fillStackFromMessage(s, t, message);
    }

    // ======== DEPTH-FIRST SEARCH ==========//

    Node * lastNode = new Node(NULL, RIGHT, 1);
    while( s->getSize() > 0 )
	{   
        if ((checkMsgCounter++ % CHECK_MSG_AMOUNT) == 0)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag)
            {
                switch (status.MPI_TAG)
                {
                    case MSG_WORK_REQUEST :
                        sendWorkTo = status.MPI_SOURCE;
                        break;
                    case MSG_TOKEN : // TODO send black token
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
			printf("Sorted! Steps: %d; bestCount: %d\n", n->steps, bestCount); // TODO: send bestCount to other processors
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
            if( toInitialSend > 0 )
            {
                sendWork(toInitialSend, n);
                toInitialSend--;
            }
            else if( sendWorkTo != -1)
            {
                sendWork(sendWorkTo, n);
                sendWorkTo = -1;
            }
            else
            {
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
    return IDLE;
}

/**
 * Processors are idle - waiting for work or something like that
 */
int idleState(Stack * s, Triangle * t, int myRank, int numberOfProcessor)
{
    int flag;
    MPI_Status status;
    char message[LENGTH];
    int attempts = 0;
    int sent = 0;
    int position = 0;

    while(true)
    {
        position = 0;
        if( !sent )
        {
            int dest;
            do
            {
                dest = rand() % numberOfProcessor; // generating destination randomly except to yourself
            }
            while( dest != myRank);
            MPI_Send( (void*) NULL, position, MPI_CHAR, dest, MSG_WORK_REQUEST, MPI_COMM_WORLD );
            sent = 1;
        }
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag)
        {
            switch (status.MPI_TAG)
            {
                case MSG_WORK_SENT : // accept work and switch to workState
                    MPI_Recv( message, LENGTH, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
                    fillStackFromMessage(s, t, message);
                    return WORK;
                case MSG_WORK_NOWORK : // ask some other cpu for work, or if attemps == numberOfProcessors-1 and myrank == 0 sent white token
                    attempts++;
                    if( attempts >= numberOfProcessor-1)
                    {
                        if (myRank == 0)
                        {
                            return TOKEN;
                        }
                    }
                    sent = 0;
                    break;
                case MSG_FINISH : // finish, switch to finish state
                                        return FINISH;
                case MSG_TOKEN : // if token is black send black else send white token to cpu+1, if myrank==0 and token is white send finish and switch to finish state
                                break;
                default : printf("neznamy typ zpravy!\n"); break;
            }
        }
    }
}

// called only when myRank set to 0
int tokenState(Stack * s, Triangle * t)
{
    // send token to next processor and wait for reply
    //             MPI_Send( (void*) NULL, position, MPI_CHAR, dest , MSG_WORK_REQUEST, MPI_COMM_WORLD );
}

int main( int argc, char** argv )
{
    srand(time(NULL));

	/* MPI VARIABLES */
	int myRank;
	int numberOfProcessors;
	int tag = 1;
	int flag;
	MPI_Status status;
	int position = 0;

	/* start up MPI */
	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcessors);

	/* Sequential Variables */
	int bestCount;
	Triangle * t;
	unsigned int microseconds = 100000;

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

    bestCount = q;

	printf("Hello i am CPU #%d. q is %d, n is %d \n", myRank, q, n);

	if( myRank == 0 )
	{
		printf("There are %d processors. \n", numberOfProcessors);
		//INIT ARGS AND TRIANGLE AND SEND TO OTHER PROCESSES

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
		char * message = t->pack(&position);
		for (int destination = 1; destination < numberOfProcessors; destination++ )
		{
			MPI_Send( (void*) message, position, MPI_PACKED, destination, tag, MPI_COMM_WORLD );
		}
	}
	else
	{
		MPI_Status status;
		int flag = 0;
		// WAIT FOR SHUFFLED TRIANGLE
		while (!flag)
		{
			MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status); // busy waiting(?)
		}

		t = new Triangle(n);

		char message[LENGTH]; // todo: dynamic length?
		MPI_Recv( message, LENGTH, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
		t->unpack(message);

        if ( DEBUG )
        {
            printf("%d: triangle:", myRank);
            t->print();
            printf("\n");
        }
	}

	Stack * s = new Stack;

    int toInitialSend = 0;
    if( myRank == 0 )
    {
        toInitialSend = numberOfProcessors - 1;
        // first nodes inserted to stack
        for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
        {
            Direction direction = Direction(dir);
            Node * initialNode = new Node(NULL, direction, 1);
            s->push( initialNode );
        }
    }

    int nextState = WORK;
    do
    {
        switch (nextState)
        {
            case WORK:
                nextState = workState(s, toInitialSend, t, myRank, bestCount);
                break;
            case IDLE:
                nextState = idleState(s, t, myRank, numberOfProcessors);
                break;
            case TOKEN:
                nextState = tokenState(s, t);
                break;
        }
    }
    while( nextState != FINISH );


    // TODO gather all results
    // MPI_Finalize

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
