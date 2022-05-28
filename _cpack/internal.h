#if !defined(MN__APARSE_INTERNAL_H)
#define MN__APARSE_INTERNAL_H

#include "api.h"

/* bits/math/implies */
#define MN__IMPLIES(a, b) (!(a) || b)

/* bits/util/exports */
#if !defined(MN__SPLIT_BUILD)
#define MN_INTERNAL static
#else
#define MN_INTERNAL extern
#endif

#define MN_INTERNAL_DATA static

/* bits/util/preproc/token_paste */
#define MN__PASTE_0(a, b) a ## b
#define MN__PASTE(a, b) MN__PASTE_0(a, b)

/* bits/util/static_assert */
#define MN__STATIC_ASSERT(name, expr) char MN__PASTE(mn__, name)[(expr)==1]

/* bits/util/ntstr/len */
MN_INTERNAL mn_size mn__slen(mn_char* s);

/* bits/util/unused */
#define MN__UNUSED(x) ((void)(x))

#endif /* MN__APARSE_INTERNAL_H */
