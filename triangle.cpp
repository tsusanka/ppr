#include "triangle.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

#define EMPTY 0
#define INVALID_MOVE -1
#define VALID_MOVE 0

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
 * Fills triangle with integers, no blank space yet.
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

/*
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

/*
 * Calculates D(X), the distance between empty and top
 */
int Triangle::getDistanceX()
{
    return emptyX+emptyY; //TODO, fix. distances of all numbers from their target positions should be added
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
        
        if( tries++ > 30 ){ //stop after 30 tries
            printf("Something is wrong, no direction seems valid");
            return;
        }
        
    } while( move(test) == INVALID_MOVE );
}

int Triangle::moveBack(Direction where)
{
    switch (where){
        case LEFT: return move(RIGHT);
        case RIGHT: return move(LEFT);
        case BOTTOM_LEFT: return move(TOP_RIGHT);
        case BOTTOM_RIGHT: return move(TOP_LEFT);
        case TOP_LEFT: return move(TOP_LEFT);
        case TOP_RIGHT: return move(BOTTOM_LEFT);
        default: 
            printf("Invalid direction");
            return INVALID_MOVE;
    }
}

int Triangle::move(Direction where)
{
    switch (where){
        case LEFT: return move(-1,0);
        case RIGHT: return move(1,0);
        case BOTTOM_LEFT: return move(0,1);
        case BOTTOM_RIGHT: return move(1,1);
        case TOP_LEFT: return move(-1,-1);
        case TOP_RIGHT: return move(-1,0);
        default: 
            printf("Invalid direction");
            return INVALID_MOVE;
    }
}
/*
    X is horizontal position in triangle, Y is vertical!!
 */
int Triangle::move(int dx, int dy)
{
    if( emptyX + dx < 0 || emptyX + dx > emptyY + dy)  //The maximum x in the Y depth in the triangle is Y
        return INVALID_MOVE;
    if( emptyY + dy < 0 || emptyY + dy >= size) 
        return INVALID_MOVE;
    
    printf("Switching EMPTY with %d at position [%d,%d]\n", array[emptyY + dy][emptyX + dx], emptyX + dx, emptyY + dy);
    
    array[emptyY][emptyX] = array[emptyY + dy][emptyX + dx];
    array[emptyY + dy][emptyX + dx] = EMPTY;
    emptyX += dx;
    emptyY += dy;
     
    return VALID_MOVE;
}


void Triangle::print()
{
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

