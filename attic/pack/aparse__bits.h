#ifndef APARSE_BITS_H
#define APARSE_BITS_H

#include "aparse_config.h"
#include "aparse__bits_api.h"

/* bit: exports */
#define APARSE_INTERNAL extern

/* bit: hook_malloc */
#if !APARSE_USE_CUSTOM_MALLOC
#include <stdlib.h>
#define APARSE_MALLOC malloc
#define APARSE_REALLOC realloc
#define APARSE_FREE free
#else
#if !defined(APARSE_MALLOC) || !defined(APARSE_REALLOC) || !defined(APARSE_FREE)
#error In order to use APARSE_USE_CUSTOM_MALLOC you must define macros for APARSE_MALLOC, APARSE_REALLOC, and APARSE_FREE.
#endif
#endif

#endif
