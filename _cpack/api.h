#if !defined(MN__APARSE_API_H)
#define MN__APARSE_API_H

#include "config.h"

/* bits/types/size */
typedef MN_SIZE_TYPE mn_size;

/* bits/util/exports */
#if !defined(MN__SPLIT_BUILD)
#if MN_STATIC
#define MN_API static
#else
#define MN_API extern
#endif
#else
#define MN_API extern
#endif

/* bits/util/null */
#define MN_NULL 0

/* bits/types/char */
#if !defined(MN_CHAR_TYPE)
#define MN_CHAR_TYPE char
#endif

typedef MN_CHAR_TYPE mn_char;

#endif /* MN__APARSE_API_H */
