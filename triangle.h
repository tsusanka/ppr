#ifndef DIRECTION_H
#define DIRECTION_H
#include "direction.h"
#endif

class Triangle
{

	protected:
		int size;
		int **array;
		void init();
		void destroy();
		int move(int dx, int dy);
		int emptyX;
		int emptyY;

	public:
		Triangle(int size);
		~Triangle();
		void print();
		void fill();
		int move(Direction where);
		Direction oppositeDirection(Direction dir);
		void randomStep();
		bool isSorted();
		int getDistanceX();
};
