#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mpi.h"
#include "main.h"

#define DEBUG_STACK false
#define DEBUG_COMM true

#define CHECK_MSG_AMOUNT  100
#define LENGTH 1000
#define SHORT_BUFFER_LENGTH 10


#define MSG_WORK_REQUEST 1000
#define MSG_WORK_SENT    1001
#define MSG_WORK_NOWORK  1002
#define MSG_TOKEN_BLACK  1003
#define MSG_TOKEN_WHITE  1004
#define MSG_FINISH       1005
#define MSG_NEW_BEST_SOLUTION 1006
#define MSG_FINISH_WITH_SOLUTION 1007
#define MSG_SHUFFLED_TRIANGLE 1008
#define MSG_FINISH_WITHOUT_SOLUTION 1009

#define WORK       8004
#define IDLE       8005
#define FINISH       8006

#define TOKEN       8007

struct Globals
{
    int bestCount;
    int solutionFound;
    int myRank;
    int numberOfProcessors;
    char * nullBuffer;
};

Globals globals;

void printMSGFlag(int flag)
{
    switch (flag) {
        case MSG_WORK_REQUEST:
            printf("MSG_WORK_REQUEST");
            break;
        case MSG_WORK_SENT:
            printf("MSG_WORK_SENT");
            break;
        case MSG_WORK_NOWORK:
            printf("MSG_WORK_NOWORK");
            break;
        case MSG_TOKEN_BLACK:
            printf("MSG_TOKEN_BLACK");
            break;
        case MSG_TOKEN_WHITE:
            printf("MSG_TOKEN_WHITE");
            break;
        case MSG_FINISH:
            printf("MSG_FINISH");
            break;
        case MSG_NEW_BEST_SOLUTION:
            printf("MSG_NEW_BEST_SOLUTION");
            break;
        case MSG_FINISH_WITH_SOLUTION:
            printf("MSG_FINISH_WITH_SOLUTION");
            break;
        case MSG_FINISH_WITHOUT_SOLUTION:
            printf("MSG_FINISH_WITHOUT_SOLUTION");
            break;
        case MSG_SHUFFLED_TRIANGLE:
            printf("MSG_SHUFFLED_TRIANGLE");
            break;
        default:
            printf("X01: #%d: aaay caramba, i don't know flag %d\n", globals.myRank, flag);
            break;
    }
}

int send(const void *buffer, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
{
    if (DEBUG_COMM)
    {
        printf("X02: #%d: I am sending message to #%d with tag ", globals.myRank, dest);
        printMSGFlag(tag);
        printf("\n");
    }
    if( buffer == NULL ){
        buffer = globals.nullBuffer;
        count = 1;
    }
    return MPI_Send( (void*) buffer, count, datatype, dest, tag, comm );
}

int receive(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
{
    int result = MPI_Recv(buf, count, datatype, source, tag, comm, status);
    if (DEBUG_COMM)
    {
        printf("X03: #%d: I am receiving message with tag ", globals.myRank);
        printMSGFlag( status->MPI_TAG );
        printf(" from #%d\n", status->MPI_SOURCE);
    }
    return result;
}

void printDirectionSymbol(Direction dir)
{
    switch (dir)
    {
        case LEFT:
            printf("<-");
            break;
        case RIGHT:
            printf("->");
            break;
        case BOTTOM_LEFT:
            printf("./");
            break;
        case BOTTOM_RIGHT:
            printf("\\.");
            break;
        case TOP_LEFT:
            printf("|\\");
            break;
        case TOP_RIGHT:
            printf("/|");
            break;
        case NONE:
            printf("NONE");
            break;
        default:
            printf("invalid direction:%d",dir);
            break;
    }
}

void copySolution( Direction * where, Node * from )
{
	Node* node = from;
	int i = 0;
	if (node == NULL)
	{
		printf("X04: Something is wrong; no solution found.\n");
		return;
	}
	printf("X40: I am gonna print the solution just for you.\n");

	do
	{
        printDirectionSymbol(node->direction);
		where[i++] = node->direction;
		node = node->prevNode;
	}
	while (node != NULL);
	assert(i > 0);
    printf("\n");
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
    printf("X45: #%d: fillStackFromMessage> triangle:", globals.myRank);
    t->print();
    while(true)
    {
        MPI_Unpack(message, LENGTH, &position, &number, 1, MPI_INT, MPI_COMM_WORLD);
        direction = (Direction) number;
        printf("X41: #%d: fillStackFromMessage> MOVE recieved:", globals.myRank);
        printDirectionSymbol(direction);
        printf("\n");
        if( direction == NONE ) {
            break;
        }
        int result = t->move ( direction );
        printf("X43: #%d: fillStackFromMessage> moving: ", globals.myRank);
        printDirectionSymbol(direction);
        printf("\n");
        if( result == -1 )
        {
            printf("X05: #%d: fillStackFromMessage>Invalid MOVE recieved:%d\n", globals.myRank, number);
        }
        Node * n = new Node(lastNode, direction, i++);
        lastNode = n;
    }
    if (DEBUG_COMM) printf("X6: #%d: I've filled my stack. \n", globals.myRank);
    t->move( t->oppositeDirection(lastNode->direction) ); //revert the last move, because it will be done after it is popped from the stack
    printf("X42: #%d: fillStackFromMessage> revert move", globals.myRank);
    printDirectionSymbol(lastNode->direction);
    printf("\n");
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
        int a = (int) result[ri];
        MPI_Pack(&a, 1, MPI_INT, buffer, LENGTH, &position, MPI_COMM_WORLD);
    }
    while( result[ri++] != NONE);
    send( (void*) buffer, position, MPI_PACKED, to, MSG_WORK_SENT, MPI_COMM_WORLD );
    delete [] buffer;
}

void broadcastBestCount(int count) {
    char * buffer = new char[SHORT_BUFFER_LENGTH];
    int position = 0;
    MPI_Pack(&count, 1, MPI_INT, buffer, LENGTH, &position, MPI_COMM_WORLD);
    for (int i = 0; i <  globals.numberOfProcessors; ++i) {
        if ( i == globals.myRank )
        {
            continue;
        }
        send (buffer, position, MPI_PACKED, i, MSG_NEW_BEST_SOLUTION, MPI_COMM_WORLD);
    }
    delete [] buffer;
}

void sendBlackToken() {
    int position = 0;
    send (NULL, position, MPI_CHAR, (globals.myRank + 1) % globals.numberOfProcessors, MSG_TOKEN_BLACK, MPI_COMM_WORLD);
}

void sendWhiteToken() {
    int position = 0;
    send (NULL, position, MPI_CHAR, (globals.myRank + 1) % globals.numberOfProcessors, MSG_TOKEN_WHITE, MPI_COMM_WORLD);
}

void sendNoWork(int to)
{
    int position = 0;
    send (NULL, position, MPI_CHAR, to, MSG_WORK_NOWORK, MPI_COMM_WORLD);
}

/**
 * sends finish flag to all processors except itself. should be called only by myRank = 0
 */
void sendFinish()
{
    if (DEBUG_COMM) printf("X07: #%d: sending FINAL to all processors \n", globals.myRank);
    int position = 0;
    for (int i = 1; i < globals.numberOfProcessors; ++i) // intentionally from 1 because of myRank = 0
    {
        send (NULL, position, MPI_CHAR, i, MSG_FINISH, MPI_COMM_WORLD);
    }
}

/**
 * sends processor's best solution to myRank 0
 */
void sendMyBestSolution(Direction * bestSolution)
{
    char * buffer = new char[LENGTH];
    int position = 0;
    assert(globals.bestCount > 0);
    assert(globals.myRank > 0);
    for(int i = 0; i < globals.bestCount; i++)
    {
        int a = (int) bestSolution[i];
        MPI_Pack(&a, 1, MPI_INT, buffer, LENGTH, &position, MPI_COMM_WORLD);
    }
    int a = -1;
    if (DEBUG_COMM) printf("X08: #%d: I'm sending my best solution to #0. \n", globals.myRank);
    MPI_Pack(&a, 1, MPI_INT, buffer, LENGTH, &position, MPI_COMM_WORLD);
    send( (void*) buffer, position, MPI_PACKED, 0, MSG_FINISH_WITH_SOLUTION, MPI_COMM_WORLD );
}


void sendNoSolutionFound() {
    int position = 0;
    if (DEBUG_COMM) printf("X09: #%d: I'm sending no solution to #0. \n", globals.myRank);
    send(  NULL, position, MPI_CHAR, 0, MSG_FINISH_WITHOUT_SOLUTION, MPI_COMM_WORLD );
}


Direction * unpackBestSolution(char * message, int* size)
{
    int position = 0;
    int number = 0;
    *size = 0;
    Direction direction;
    Direction * result = new Direction[LENGTH];
    while(true)
    {
        MPI_Unpack(message, LENGTH, &position, &number, 1, MPI_INT, MPI_COMM_WORLD);
        direction = (Direction) number;
        if( DEBUG_COMM ){
            printf("X41: #%d: unpackBestSolution> DIRECTION recieved:", globals.myRank);
            printDirectionSymbol(direction);
            printf("\n");
        }
        if( direction == -1 )
        {
            break;
        }
        result[*size] = direction;
        (*size)++;
    }
    return result;
}

int recieveBestCount( char * message )
{
    int position = 0;
    int recievedCount = 0;
    MPI_Unpack(message, LENGTH, &position, &recievedCount, 1, MPI_INT, MPI_COMM_WORLD);
    return recievedCount;
}

void receiveBestSolution(char * buffer)
{
    int newCount = recieveBestCount(buffer);
    if (DEBUG_COMM) printf("X11: #%d: I've received new best solution with steps count %d \n", globals.myRank, newCount);
    if (newCount < globals.bestCount)
    {
        if (DEBUG_COMM) printf("X12: #%d: New bestCount (%d) is better than the old count (%d) \n", globals.myRank, newCount, globals.bestCount);
        globals.bestCount = newCount;
        globals.solutionFound = 0;
    }
}

/**
 * Deals with all the work
 */
int workState( Stack * s, int toInitialSend, Triangle * t, Direction * bestSolution)
{
    int checkMsgCounter = 0;
    int flag;
    MPI_Status status;
    int sendWorkTo = -1;
    int newCount = 0;
    char * buffer;

    // ======== DEPTH-FIRST SEARCH ==========//

    Node * lastNode = new Node(NULL, RIGHT, 1);
    while( s->getSize() > 0 )
	{   
        if ((checkMsgCounter++ % CHECK_MSG_AMOUNT) == 0)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag)
            {
                char * buffer = new char[SHORT_BUFFER_LENGTH];
                MPI_Status status;
                receive(buffer, SHORT_BUFFER_LENGTH, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if (DEBUG_COMM)
                {
                    printf("X10: #%d: I've received a message with tag ", globals.myRank);
                    printMSGFlag(status.MPI_TAG);
                    printf("\n");
                }
                switch (status.MPI_TAG)
                {
                    case MSG_WORK_REQUEST :
                        sendWorkTo = status.MPI_SOURCE;
                        break;
                    case MSG_TOKEN_BLACK :
                    case MSG_TOKEN_WHITE:
                        sendBlackToken();
                        break;
                    case MSG_NEW_BEST_SOLUTION:
                        receiveBestSolution(buffer);
                        break;
                    case MSG_WORK_NOWORK:
                        break;
                    default:
                        printf("X13: #%d: neznamy typ zpravy! tag %d\n", globals.myRank, status.MPI_TAG);
                        break;
                }
                delete buffer;
            }
        }
		Node* n = s->pop();

		if(DEBUG_STACK)
		{
			printf("X14: #%d: I am popping: ", globals.myRank);
			t->printDirectionSymbol(n->direction);
			printf("\n");
		}

		while( lastNode->prevNode != NULL && n->steps < lastNode->steps)
		{
			if(DEBUG_STACK) printf("X15: #%d: dead end, reverting parent\n", globals.myRank);
			t->move( t->oppositeDirection(lastNode->prevNode->direction) ); // revert parent move

			Node * lastNodePrevNode = lastNode->prevNode;
			lastNode->direction = lastNodePrevNode->direction;   // lastNode = lastNodePrevNode
			lastNode->steps = lastNodePrevNode->steps;
			lastNode->prevNode = lastNodePrevNode->prevNode;
			delete lastNodePrevNode;
		}


		if( n->prevNode != NULL && n->prevNode->direction == t->oppositeDirection(n->direction) ) // simple optimization, don't make moves there and back
		{
			if(DEBUG_STACK) printf("X16: #%d: opposite move, skipping\n", globals.myRank);
			delete n;
			continue;
		}

		if( t->move(n->direction) == -1) // INVALID_MOVE
		{
			if(DEBUG_STACK) printf("X17: #%d: invalid move, skipping\n", globals.myRank);
			delete n;
			continue;
		}

		lastNode->direction = n->direction;  // lastNode = lastNode->prevNode
		lastNode->prevNode = n->prevNode;
		lastNode->steps = n->steps;

		if(DEBUG_STACK) printf("X18: #%d: steps: %d\n", globals.myRank, n->steps);

		if( t->isSorted() ) // this is a solution
		{
			printf("X19: #%d: Sorted! Steps: %d; globals.bestCount: %d\n", globals.myRank, n->steps, globals.bestCount); // TODO: send bestCount to other processors
			if( n->steps <= globals.bestCount )
			{
				globals.bestCount = n->steps;
				printf("X20: #%d: New solution found with %d steps\n", globals.myRank, globals.bestCount);
				copySolution( bestSolution, n);
                broadcastBestCount(globals.bestCount);
                globals.solutionFound = 1;
			}
			t->move( t->oppositeDirection(n->direction) ); // revert last move
			// todo revert parent
			continue;
		}

		if( n->steps < globals.bestCount )
		{
            if( toInitialSend > 0 )
            {
                if (DEBUG_COMM) printf("X21: #%d: I am sending INITIAL send work to #%d \n", globals.myRank, toInitialSend);
                sendWork(toInitialSend, n);
                toInitialSend--;
                t->move( t->oppositeDirection(n->direction) ); // revert last move
            }
            else if( sendWorkTo != -1)
            {
                if (DEBUG_COMM) printf("X22: #%d: I am sending usual work to #%d \n", globals.myRank, sendWorkTo);
                sendWork(sendWorkTo, n);
                sendWorkTo = -1;
                t->move( t->oppositeDirection(n->direction) ); // revert last move
            }
            else
            {
                if( DEBUG_STACK) printf("X23: #%d: inserting moves\n", globals.myRank);
                for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
                {
                    Direction direction = Direction(dir);
                    s->push( new Node(n, direction, n->steps + 1 ));
                }
            }
		}
		else
		{
			if( DEBUG_STACK) printf("X24: #%d: reverting move\n", globals.myRank);
			t->move( t->oppositeDirection(n->direction) ); // revert last move
			delete n;
		}

	}
    delete lastNode;
    return IDLE;
}

/**
 * Processors are idle - waiting for work or something like that
 */
int idleState(Stack * s, Triangle * t)
{
    if(globals.numberOfProcessors == 1) return FINISH;
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
                dest = rand() % globals.numberOfProcessors; // generating destination randomly except to yourself
            }
            while( dest == globals.myRank);
            send( (void*) NULL, position, MPI_CHAR, dest, MSG_WORK_REQUEST, MPI_COMM_WORLD );
            sent = 1;
        }
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag)
        {
            if (status.MPI_TAG == MSG_WORK_SENT)
            {
                receive( message, LENGTH, MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
            }
            else
            {
                receive( message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
            }
            switch (status.MPI_TAG)
            {
                case MSG_WORK_SENT : // accept work and switch to workState
                    fillStackFromMessage(s, t, message);
                    return WORK;
                case MSG_WORK_NOWORK : // ask some other cpu for work, or if attemps == globals.numberOfProcessors-1 and myrank == 0 sent white token
                    attempts++;
                    if( attempts >= globals.numberOfProcessors - 1 && globals.myRank == 0) // number of other processors
                    {
                        return TOKEN;
                    }
                    sent = 0;
                    break;
                case MSG_FINISH : // finish, switch to finish state
                    return FINISH;
                case MSG_TOKEN_BLACK : // if token is black send black else send white token to cpu+1, if myrank==0 and token is white send finish and switch to finish state
                    sendBlackToken();
                    break;
                case MSG_TOKEN_WHITE:
                    sendWhiteToken();
                    break;
                case MSG_NEW_BEST_SOLUTION:
                    receiveBestSolution(message);
                    break;
                case MSG_WORK_REQUEST:
                    sendNoWork(status.MPI_SOURCE);
                    break;
                default : printf("X25: #%d: neznamy typ zpravy, tag %d!\n", globals.myRank, status.MPI_TAG); break;
            }
        }
    }
}

// called only when myRank set to 0
int tokenState()
{
    int flag;
    MPI_Status status;
    sendWhiteToken();
    char * buffer = new char[SHORT_BUFFER_LENGTH];
    while(true)
    {
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag)
        {
            receive( buffer, SHORT_BUFFER_LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status );
            switch (status.MPI_TAG)
            {
                case MSG_TOKEN_BLACK : // if token is black send black else send white token to cpu+1, if myrank==0 and token is white send finish and switch to finish state
                    return IDLE;
                case MSG_TOKEN_WHITE:
                    return FINISH;
                case MSG_WORK_REQUEST:
                    sendNoWork(status.MPI_SOURCE);
                    break;
                case MSG_WORK_NOWORK:
                    break;
                default :
                    printf("X26: #%d: unknown tag %d.\n", globals.myRank, status.MPI_TAG);
                    break;
            }
        }
    }

}

int main( int argc, char** argv )
{
    srand(time(NULL));

	/* MPI VARIABLES */
	int tag = 9999;
    globals.nullBuffer = new char[1];
    globals.nullBuffer[0] = -1;
	MPI_Status status;
	int position = 0;

	/* start up MPI */
	MPI_Init(&argc, &argv);

	/* find out process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &(globals.myRank));

	/* find out number of processes */
	MPI_Comm_size(MPI_COMM_WORLD, &(globals.numberOfProcessors));

	/* Sequential Variables */
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

    globals.bestCount = q;

	printf("Hello i am CPU #%d. q is %d, n is %d \n", globals.myRank, q, n);

	if( globals.myRank == 0 )
	{
		printf("X27: #0: i can tell you there are %d processors. \n", globals.numberOfProcessors);

		t = new Triangle(n);
		t->fill();
		printf("X28: #0: Default triangle:\n");
		t->print();

//        for( int i = 0; i < q; i++ )
//		{
//			t->randomStep();
//		}
//        t->move(BOTTOM_RIGHT);
        t->move(BOTTOM_RIGHT);
//        t->move(LEFT);

		printf("X29: #0: Triangle after shuffle:\n");
		t->print();
		printf("==============================\n");

		// SEND
		char * message = t->pack(&position);
		for (int destination = 1; destination < globals.numberOfProcessors; destination++ )
		{
			send( (void*) message, position, MPI_PACKED, destination, MSG_SHUFFLED_TRIANGLE, MPI_COMM_WORLD );
		}
        delete [] message;
	}
	else
	{
		int flag = 0;
		// WAIT FOR SHUFFLED TRIANGLE
		while (!flag)
		{
			MPI_Iprobe(0, MSG_SHUFFLED_TRIANGLE, MPI_COMM_WORLD, &flag, &status); // busy waiting(?)
		}

		t = new Triangle(n);

		char message[LENGTH]; // todo: dynamic length?
		receive( message, LENGTH, MPI_PACKED, 0, MSG_SHUFFLED_TRIANGLE, MPI_COMM_WORLD, &status );
		t->unpack(message);

        if (DEBUG_COMM)
        {
            printf("X30: #%d: triangle:", globals.myRank);
            t->print();
            printf("\n");
        }
	}

	Stack * s = new Stack;

    int toInitialSend = 0;
    if( globals.myRank == 0 )
    {
        toInitialSend = globals.numberOfProcessors - 1;
        // first nodes inserted to stack
        for ( int dir = TOP_LEFT; dir <= BOTTOM_RIGHT; dir++ )
        {
            Direction direction = Direction(dir);
            Node * initialNode = new Node(NULL, direction, 1);
            s->push( initialNode );
        }
    }
    else // sends first ever work
    {
        printf("X38: #%d: I'm waiting for initial work \n", globals.myRank);
        int flag = 0;
        char message[LENGTH]; // TODO dynamic?
        while (!flag)
        {
            MPI_Iprobe(0, MSG_WORK_SENT, MPI_COMM_WORLD, &flag, &status);
        }
        printf("X37: #%d: Probe request flag true, recieving \n", globals.myRank);
        receive( message, LENGTH, MPI_PACKED, 0, MSG_WORK_SENT, MPI_COMM_WORLD, &status );
        fillStackFromMessage(s, t, message);
    }

    Direction * bestSolution = new Direction[globals.bestCount];
    int nextState = WORK;
    do
    {
        switch (nextState)
        {
            case WORK:
                if (DEBUG_COMM) printf("X31: #%d: I am changing my state to WORK. \n", globals.myRank);
                nextState = workState(s, toInitialSend, t, bestSolution);
                break;
            case IDLE:
                if (DEBUG_COMM) printf("X32: #%d: I am changing my state to IDLE. \n", globals.myRank);
                nextState = idleState(s, t);
                break;
            case TOKEN:
                if (DEBUG_COMM) printf("X33: #%d: I am changing my state to TOKEN. \n", globals.myRank);
                nextState = tokenState();
                break;
        }
    }
    while( nextState != FINISH );
    if (DEBUG_COMM) printf("X34: #%d: I am changing my state to FINISH. \n", globals.myRank);

    if( globals.myRank == 0 )
    {
        if( globals.numberOfProcessors > 1 ) {
            sendFinish();

            Direction *tempBestSolution;

            char message[LENGTH];
            int size = 0;
            for (int source = 1; source < globals.numberOfProcessors;)
            {
                int flag = 0;
                /* checking if message has arrived */
                MPI_Iprobe(MPI_ANY_SOURCE, MSG_FINISH_WITH_SOLUTION, MPI_COMM_WORLD, &flag, &status);
                if (flag)
                {
                    /* receiving message by blocking receive */
                    receive(&message, LENGTH, MPI_PACKED, MPI_ANY_SOURCE, MSG_FINISH_WITH_SOLUTION, MPI_COMM_WORLD, &status); // MPI_INT -> MPI_PACKED?
                    if (message != NULL)
                    {
                        tempBestSolution = unpackBestSolution(message, &size);
                        if (DEBUG_COMM) printf("X43: #%d: solution received with %d size\n", globals.myRank, size);
                        if (size <= globals.bestCount)
                        {
                            delete bestSolution;
                            globals.bestCount = size;
                            bestSolution = tempBestSolution;
                            if (DEBUG_COMM) printf("X35: #%d: Found better solution \n", globals.myRank);
                        }
                        else
                        {
                            delete[] tempBestSolution;
                        }
                    }
                    source++;
                }
                MPI_Iprobe(MPI_ANY_SOURCE, MSG_FINISH_WITHOUT_SOLUTION, MPI_COMM_WORLD, &flag, &status);
                if (flag)
                {
                    receive(&message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MSG_FINISH_WITHOUT_SOLUTION, MPI_COMM_WORLD, &status); //just continue
                    source++;
                }

            }
        }
    }
    else if( globals.solutionFound )
    {
        sendMyBestSolution(bestSolution);
        MPI_Finalize();
        return 0;
    }else{
        sendNoSolutionFound();
        MPI_Finalize();
        return 0;
    }

	printf("==============================\n");
	printf("X36: End: best solution found with %d steps. Moves:\n", globals.bestCount);
	for( int i = globals.bestCount - 1; i >= 0; i-- )
	{
		t->printDirectionSymbol(bestSolution[i]);
	}
    printf("\n");

    MPI_Finalize();

	delete [] bestSolution;
    delete s;
    delete t;
    delete [] globals.nullBuffer;

	return 0;
}
