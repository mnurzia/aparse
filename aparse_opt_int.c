
#include "aparse_internal.h"

MN_INTERNAL int aparse__arg_cb_int(aparse_state* state, int sub_arg_idx, void* data, const char* arg)
{
    int target;
    (void)(sub_arg_idx);
    if (sscanf(arg, "%i", &target) != 1) {
        aparse__error_begin(state);
        aparse__error_arg_begin(state);
        fprintf(stderr, "invalid int value: '%s'", arg);
        aparse__error_arg_end(state);
        aparse__error_end(state);
        return 1;
    }
    *((int*)(data)) = target;
    return 0;
}

MN_API int aparse_opt_int(aparse_state* state, int* out)
{
    state->current_arg->nargs = 1;
    state->current_arg->callback = aparse__arg_cb_int;
    state->current_arg->callback_data = (void*)out;
    return 0;
}
