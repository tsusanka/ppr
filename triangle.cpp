#include "triangle.h"
#include <stdio.h>

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
			array[i][y] = ++cnt;
		}
	}
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
			printf(" %02d ", array[i][y]);
		}
		printf("\n");
	}
}

