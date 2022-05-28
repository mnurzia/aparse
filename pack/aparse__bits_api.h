#ifndef APARSE_BITS_API_H
#define APARSE_BITS_API_H

#include "aparse_config.h"

/* bit: version */
#define APARSE_VERSION_MAJOR 0
#define APARSE_VERSION_MINOR 0
#define APARSE_VERSION_PATCH 1
#define APARSE_VERSION_STRING "0.0.1"

/* bit: exports */
#if APARSE_STATIC
    #define APARSE_API static
#else
    #define APARSE_API extern
#endif

/* bit: unused */
#define APARSE_UNUSED(i) (void)(i)

#endif
