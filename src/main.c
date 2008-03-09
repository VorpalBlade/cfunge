#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include "funge-space/b93/funge-space.h"

static fungeSpace *space;

int main(int argc, char *argv[])
{
	GC_INIT();
	space = fungeSpaceCreate();
	if (argc > 0)
		fungeSpaceLoad(space, argv[1]);
	return EXIT_SUCCESS;
}
