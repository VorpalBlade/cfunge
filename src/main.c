#include "global.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "funge-space/b93/funge-space.h"

static fungeSpace *space;

int main(int argc, char *argv[])
{
	GC_INIT();
	// No proper command line parsing I know.
	if (argc > 1) {
		bool retval;
		space = fungeSpaceCreate();
		retval = fungeSpaceLoad(space, argv[1]);
		if (!retval) {
			fprintf(stderr, "Failed to handle file: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	} else {
		fprintf(stderr, "Usage %s file\n", argv[0]);
		return EXIT_FAILURE;
	}

}
