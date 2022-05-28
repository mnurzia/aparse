#if !defined(MN__APARSE_CONFIG_H)
#define MN__APARSE_CONFIG_H

/* desc */
/* cppreference */
#if !defined(MN_ASSERT)
#include <assert.h>
#define MN_ASSERT assert
#endif

/* bits/hooks/malloc */
/* Set to 1 in order to define malloc(), free(), and realloc() replacements. */
#if defined(MN_USE_CUSTOM_ALLOCATOR)
#define MN_MALLOC my_malloc
#define MN_REALLOC my_realloc
#define MN_FREE my_free
#else
#include <stdlib.h>
#define MN_MALLOC malloc
#define MN_REALLOC realloc
#define MN_FREE free
#endif

/* bits/types/size */
/* desc */
/* cppreference */
#if !defined(MN_SIZE_TYPE)
#include <stdlib.h>
#define MN_SIZE_TYPE size_t
#endif

/* bits/util/exports */
/* Set to 1 in order to define all symbols with static linkage (local to the
 * including source file) as opposed to external linkage. */
#if !defined(MN_STATIC)
#define MN_STATIC 0
#endif

/* bits/types/char */
/* desc */
/* cppreference */
#if !defined(MN_CHAR_TYPE)
#define MN_CHAR_TYPE char
#endif

#endif /* MN__APARSE_CONFIG_H */
