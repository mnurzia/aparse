#ifndef APARSE_CONFIG_H
#define APARSE_CONFIG_H

/* config: USE_MALLOC */
/* Whether or not to use malloc. */
#ifndef APARSE_USE_MALLOC
#define APARSE_USE_MALLOC 1
#endif

/* config: STATIC */
/* Whether or not API definitions should be defined as static linkage (local to
 * the including source file), as opposed to external linkage. */
#ifndef APARSE_STATIC
#define APARSE_STATIC 0
#endif

/* config: USE_CUSTOM_MALLOC */
/* Set to 1 in order to use your own allocator for all allocations. You must define APARSE_MALLOC, APARSE_REALLOC, and APARSE_FREE if so. If unset, <stdlib.h> is included and malloc(), realloc(), and free() are used. */
#ifndef APARSE_USE_CUSTOM_MALLOC
#define APARSE_USE_CUSTOM_MALLOC 0
#endif

/* config: MALLOC */
/* malloc function to hook */
#if APARSE_USE_CUSTOM_MALLOC
#ifndef APARSE_MALLOC
#define APARSE_MALLOC malloc
#endif
#endif

/* config: REALLOC */
/* free function to hook */
#if APARSE_USE_CUSTOM_MALLOC
#ifndef APARSE_REALLOC
#define APARSE_REALLOC realloc
#endif
#endif

/* config: FREE */
/* realloc function to hook */
#if APARSE_USE_CUSTOM_MALLOC
#ifndef APARSE_FREE
#define APARSE_FREE free
#endif
#endif

#endif
