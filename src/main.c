#include "global.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	GC_INIT();
	printf("Hello, world!\n");

	return EXIT_SUCCESS;
}
