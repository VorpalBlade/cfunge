#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "interpreter.h"


int main(int argc, char *argv[])
{
	GC_INIT();
	// No proper command line parsing I know.
	if (argc > 1) {
		return interpreterRun(argc, argv);
	} else {
		fprintf(stderr, "Usage %s file\n", argv[0]);
		return EXIT_FAILURE;
	}

}
