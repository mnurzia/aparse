#include "aparse_internal.h"

MN_API void aparse_init_fixed(aparse_state* state, struct aparse__arg* opts,
    size_t opts_alloc)
{
    state->help        = NULL;
    state->version     = NULL;
    state->args        = opts;
    state->args_size   = 0;
    state->args_alloc  = opts_alloc;
    state->current_arg = state->args;
    state->in_arg      = 0;
    state->argc        = 0;
    state->argv        = NULL;
}

#if APARSE_USE_MALLOC
APARSE_API int aparse_init(aparse_state* state)
{
    struct aparse__arg* opts
        = (struct aparse__arg*)APARSE_MALLOC(sizeof(struct aparse__arg) * 16);
    if (!opts) {
        return 1;
    }
    aparse_init_fixed(state, opts, 16);
    return 0;
}
#endif

MN_API void aparse_destroy(aparse_state* state)
{
#if APARSE_USE_MALLOC
    if (state->args) {
        free(state->args);
    }
#else
    (void)(state);
#endif
}

MN_API int aparse_add_opt(aparse_state* state, char short_opt, const char* long_opt)
{
    if (state->args_size == state->args_alloc) {
#if APARSE_USE_MALLOC
        size_t current_opt_idx = (size_t)(state->current_arg - state->args);
        state->args_alloc *= 2;
        state->args = (struct aparse__arg*)realloc(state->args,
            sizeof(struct aparse__arg*) * state->args_alloc);
        if (!state->args) {
            return 1;
        }
        state->current_arg = state->args + current_opt_idx;
#else
        return 1;
#endif
    }
    /* Advance option pointer if this isn't the first option */
    if (state->in_arg != 0) {
        state->current_arg++;
    } else {
        state->in_arg = 1;
    }
    aparse__arg_init(state->current_arg, short_opt, long_opt);
    state->args_size++;
    return 0;
}
