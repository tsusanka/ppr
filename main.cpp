#include <stdio.h>
#include "triangle.h"

main()
{
	Triangle *t = new Triangle(5);
	t->fill();
	t->print();
}

