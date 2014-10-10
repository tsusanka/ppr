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
		row = new int[size+1];
		row[0] = 0;
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

void Triangle::print()
{
	for (int i = 0; i < size; i++)
	{
		for (int y = 0; y < size; y++)
		{
			printf("%d | ", array[i][y]);
		}
	}
}

