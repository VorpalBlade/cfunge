#include "global.h"
#include "support.h"

#include <string.h>

char * cf_strndup(char const *string, size_t n)
{
	if (!string || !*string)
		return NULL;
	// Keep gcc happy with variable decls
	{
		size_t len = strnlen(string, n);
		char *newstr = cf_malloc_noptr(len + 1);

		if (newstr == NULL)
			return NULL;

		newstr[len] = '\0';
		return memcpy(newstr, string, len);
	}
}
