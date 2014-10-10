#include <stdio.h>
#include <assert.h>
#include "triangle.h"

main()
{
	int n = 0, q = 0;
	printf("Enter triangle size (n):");
	scanf("%d", &n);
	assert(n > 0);
	printf("Enter random steps count (q):");
	scanf("%d", &q);
	assert(q > 0);

	Triangle *t = new Triangle(n);
	t->fill();
	t->print();
}

