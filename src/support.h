// Do not include directly, include global.h instead. It includes
// this file.
#ifndef _HAD_SRC_SUPPORT_H
#define _HAD_SRC_SUPPORT_H

#include <gc/gc.h>

#define cf_malloc(x)           GC_MALLOC(x)
// Use this for strings and other stuff containing no pointers when possible.
#define cf_malloc_noptr(x)     GC_MALLOC_ATOMIC(x)
// This memory is not collectable. Avoid using this unless you have to.
#define cf_malloc_nocollect(x) GC_MALLOC_UNCOLLECTABLE(x)
#define cf_free(x)             GC_FREE(x)
#define cf_realloc(x,y)        GC_REALLOC(x, y)
#define cf_calloc(x,y)         GC_MALLOC((x)*(y))
#define cf_calloc_noptr(x,y)   GC_MALLOC_ATOMIC((x)*(y))

#define gc_collect_full()      GC_gcollect()
#define gc_collect_some()      GC_collect_a_little()

// Use these only if you have to, for example if some external library
// did the malloc
#define malloc_nogc(x)         malloc(x)
#define calloc_nogc(x,y)       calloc(x, y)
#define realloc_nogc(x,y)      realloc(x, y)
#define free_nogc(x)           free(x);


#define cf_strdup(x)       GC_STRDUP(x)
char * cf_strndup(char const *string, size_t n) __attribute__((warn_unused_result));

#endif
