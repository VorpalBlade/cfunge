#ifndef _HAD_SRC_GLOBAL_H
#define _HAD_SRC_GLOBAL_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

// For compatiblity with other compilers to prevent them
// failing at things like: __attribute__((noreturn))
#ifndef __GNUC__
#  define  __attribute__(x)  /* NO-OP */
#endif

#include <support.h>

#endif
