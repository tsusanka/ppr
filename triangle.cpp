#include "triangle.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "mpi.h"

#define EMPTY 0
#define INVALID_MOVE -1
#define VALID_MOVE 0
#define DEBUG false
#define LENGTH 1000

Triangle::Triangle(int s)
{
	size = s;
	init();
}

Triangle::~Triangle()
{
	destroy();
}

void Triangle::init()
{
	array = new int*[size];

	/* initialize random seed: */
	srand (time(NULL));

	for (int i = 0; i < size; i++)
	{
		int *row;
		row = new int[i + 1];
		array[i] = row;
		for (int y = 0; y <= i; y++)
		{
			array[i][y] = 0;
		}
	}
}

void Triangle::destroy()
{
	for (int i = 0; i < size; i++)
	{
		delete [] array[i];
	}
	delete [] array;
}

/**
 * Fills triangle with integers, puts EMPTY at the top.
 */
void Triangle::fill()
{
	int cnt = 0;
	for (int i = 0; i < size; i++)
	{
		for (int y = 0; y <= i; y++)
		{
			array[i][y] = cnt++;
		}
	}
	emptyX = emptyY = 0;
}

/**
 * Checks if triangle is sorted, in the same form as it was initialized by fill method 
 */
bool Triangle::isSorted()
{
	if( array[0][0] != EMPTY)
		return false;

	int cnt = 0;
	for (int i = 0; i < size; i++)
	{
		for (int y = 0; y <= i; y++)
		{
			if( array[i][y] != cnt++ ) 
				return false;
		}
	}
	return true;
}

/**
 * Calculates D(X), the distance between empty and top
 * 
 * Edux:
 *  d(X) je spodní mez počtu tahů, kterými se lze dostat z konfigurace X
 *  do cílové konfigurace C. Tato spodní mez je rovna součtu vzdáleností
 *  koleček od jejich cílových políček. Vzdálenost 2 políček v této síti
 *  se počítá takto: Jsou-li obě políčka na úsečce rovnoběžné se stranou
 *  trojúhelníka, pak je vzdálenost rovna jejich lineární vzdálenosti po
 *  této úsečce. V opačném případě tvoří políčka vrcholy kosodélníka
 *  a vzdálenost se rovná součtu délek jeho dvou stran. Spodní mez počtu
 *  tahů nejlepšího možného řešení je tedy d(X0).
 */
int Triangle::getDistanceX()
{
	return emptyX + emptyY; // TODO, fix. distances of all numbers from their target positions should be added
}

/**
 * Shifts the empty place in a random (but valid) direction
 */
void Triangle::randomStep()
{
	Direction test;
	int tries = 0;
	do
	{
		test = Direction( rand() % 6 );

		if( tries++ > 30 ) // stop after 30 tries
		{
			printf("Something is wrong, no direction seems valid");
			return;
		}
	}
	while( move(test) == INVALID_MOVE );
	printDirectionSymbol(test);
}

/*
 * returns opposite direction enum, should be probably static
 */
Direction Triangle::oppositeDirection(Direction dir)
{
	switch (dir)
	{
		case LEFT: return RIGHT;
		case RIGHT: return LEFT;
		case BOTTOM_LEFT: return TOP_RIGHT;
		case BOTTOM_RIGHT: return TOP_LEFT;
		case TOP_LEFT: return BOTTOM_RIGHT;
		case TOP_RIGHT: return BOTTOM_LEFT;
		default:
			printf("Triangle::oppositeDirection error: Invalid direction, %d", dir);
			throw -1;
	}
        return RIGHT;
}

int Triangle::move(Direction where)
{
	switch (where)
	{
		case LEFT: return move(LEFT, 0, -1); // 0 = same row; -1 = one to the left
		case RIGHT: return move(RIGHT, 0, 1);
		case BOTTOM_LEFT: return move(BOTTOM_LEFT, 1, 0);
		case BOTTOM_RIGHT: return move(BOTTOM_RIGHT, 1, 1);
		case TOP_LEFT: return move(TOP_LEFT, -1, -1);
		case TOP_RIGHT: return move(TOP_RIGHT, -1, 0);
		default:
			printf("Invalid direction");
			return INVALID_MOVE;
	}
}

int Triangle::move(Direction where, int dx, int dy) // direction only for printing purposes
{
	if( emptyY + dy < 0 || emptyY + dy > emptyX + dx){
        // The maximum y in the X depth in the triangle is X
        //printf("invalid move, size %d, emptyY %d, emptyX %d, dx %d, dy %d\n",size, emptyY, emptyX, dx, dy);
        return INVALID_MOVE;
    }
	if( emptyX + dx < 0 || emptyX + dx >= size){
        //printf("invalid move2, size %d, emptyY %d, emptyX %d, dx %d, dy %d\n",size, emptyY, emptyX, dx, dy);
        return INVALID_MOVE;
    }

	array[emptyX][emptyY] = array[emptyX + dx][emptyY + dy];
	array[emptyX + dx][emptyY + dy] = EMPTY;
	emptyY += dy;
	emptyX += dx;
        if( DEBUG ) {
            printMove(where);
        }

	return VALID_MOVE;
}

void Triangle::print()
{
    printf("print__ Triangle size %d\n", size);
	for (int i = 0; i < size; i++)
	{
		// padding
		for (int z = 0; z < size - i - 1; z++)
		{
			printf("  ");
		}
		// values
		for (int y = 0; y <= i; y++)
		{
			if (array[i][y] == EMPTY)
			{
				printf(" xx ");
			}
			else
			{
				printf(" %02d ", array[i][y]);
			}
		}
		printf("\n");
	}
}

/**
 * Should be static as well
 */
void Triangle::printDirectionSymbol(Direction dir)
{
	printDirectionSymbolWin32(dir);
	return;
}

void Triangle::printDirectionSymbolWin32(Direction dir)
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
		default:
			printf("invalid direction");
			throw -2;
	}
}

void Triangle::printMove(Direction where)
{
	printf("Moving ");
//	for (int i = 0; i < 10; ++i)
	//{
		printDirectionSymbol(where);
//	}
	printf(" result:\n");
	print();
}

char* Triangle::pack(int* position)
{
	char* buffer = new char[LENGTH];
	for (int i = 0; i < size; i++)
	{
		for (int y = 0; y <= i; y++)
		{
			int a = array[i][y];
			MPI_Pack(&a, 1, MPI_INT, buffer, LENGTH, position, MPI_COMM_WORLD);
		}
	}
	return buffer;
}

void Triangle::unpack(char* buffer)
{
	int position = 0;
	int number;
	for (int i = 0; i < size; i++)
	{
		for (int y = 0; y <= i; y++)
		{
			MPI_Unpack(buffer, LENGTH, &position, &number, 1, MPI_INT, MPI_COMM_WORLD);
			array[i][y] = number;
            if(number == 0){
                emptyX = i;
                emptyY = y;
            }
		}
	}
}
