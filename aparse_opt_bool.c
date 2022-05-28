#include "aparse_internal.h"

MN_INTERNAL int aparse__arg_cb_bool(aparse_state* state, int sub_arg_idx, void* data, const char* arg)
{
    MN__UNUSED(state);
    MN__UNUSED(sub_arg_idx);
    MN__UNUSED(arg);
    *((int*)data) = 1;
    return 0;
}

MN_API int aparse_opt_bool(aparse_state* state, int* out)
{
    state->current_arg->nargs = APARSE_NARGS_0_OR_1_EQ;
    state->current_arg->callback = aparse__arg_cb_bool;
    state->current_arg->callback_data = (void*)out;
    return 0;
}
