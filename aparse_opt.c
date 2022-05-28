#include "aparse_internal.h"

MN_INTERNAL void aparse__arg_init(struct aparse__arg* opt, char short_opt,
    const char* long_opt)
{
    opt->short_opt     = short_opt;
    opt->long_opt      = long_opt;
    opt->description   = NULL;
    opt->metavar       = NULL;
    opt->callback      = NULL;
    opt->callback_data = NULL;
    opt->type          = APARSE__ARG_TYPE_OPTIONAL;
    opt->nargs         = 0;
    opt->required      = 0;
}

MN_API void aparse_opt_desc(aparse_state* state, const char* desc)
{
    state->current_arg->description = desc;
}

MN_API void aparse_opt_nargs(aparse_state* state, aparse_nargs nargs)
{
    state->current_arg->nargs = nargs;
}

MN_API void aparse_opt_metavar(aparse_state* state, const char* metavar)
{
    state->current_arg->metavar = metavar;
}
