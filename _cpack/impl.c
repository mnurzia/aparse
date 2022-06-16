#include "../tests/test_config.h"
#include "internal.h"

/* bits/types/char */
MN__STATIC_ASSERT(mn__char_is_one_byte, sizeof(mn_char) == 1);

/* bits/util/ntstr/len */
MN_INTERNAL mn_size mn__slen(const mn_char* s) {
    mn_size sz = 0;
    while (*s) {
        sz++;
        s++;
    }
    return sz;
}

