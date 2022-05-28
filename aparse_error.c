#include "aparse_internal.h"

MN_INTERNAL void aparse__error_print_opt_name(aparse_state* state,
    const struct aparse__arg*                   opt)
{
    (void)(state);
    if (opt->short_opt && !opt->long_opt) {
        /* Only short option was specified */
        fprintf(stderr, "-%c", opt->short_opt);
    } else if (opt->short_opt && opt->long_opt) {
        /* Both short option and long option were specified */
        fprintf(stderr, "-%c/--%s", opt->short_opt, opt->long_opt);
    } else if (opt->long_opt) {
        /* Only long option was specified */
        fprintf(stderr, "--%s", opt->long_opt);
    }
}

MN_INTERNAL void aparse__error_print_usage_coalesce_short_args(aparse_state* state)
{
    /* Print optional short options without arguments */
    size_t i;
    int    has_printed_short_opt_no_arg = 0;
    for (i = 0; i < state->args_size; i++) {
        const struct aparse__arg* current_opt = &state->args[i];
        /* Filter out positional options */
        if (current_opt->type == APARSE__ARG_TYPE_POSITIONAL) {
            continue;
        }
        /* Filter out required options */
        if (current_opt->required) {
            continue;
        }
        /* Filter out options with no short option */
        if (!current_opt->short_opt) {
            continue;
        }
        /* Filter out options with nargs that don't coalesce */
        if (!aparse__nargs_can_coalesce(current_opt->nargs)) {
            continue;
        }
        /* Print the initial '[-' */
        if (!has_printed_short_opt_no_arg) {
            has_printed_short_opt_no_arg = 1;
            fprintf(stderr, " [-");
        }
        fprintf(stderr, "%c", current_opt->short_opt);
    }
    if (has_printed_short_opt_no_arg) {
        fprintf(stderr, "]");
    }
}

MN_INTERNAL void aparse__error_print_usage_arg(aparse_state* state,
    const struct aparse__arg*                    current_arg)
{
    const char* metavar = "ARG";
    (void)(state);
    if (current_arg->metavar) {
        metavar = current_arg->metavar;
    }
    /* Optional arguments are encased in [] */
    if (!current_arg->required) {
        fprintf(stderr, "[");
    }
    /* Print option name */
    if (current_arg->type == APARSE__ARG_TYPE_OPTIONAL) {
        if (current_arg->short_opt) {
            fprintf(stderr, "-%c", current_arg->short_opt);
        } else {
            fprintf(stderr, "--%s", current_arg->long_opt);
        }
        /* Space separates the option name from the arguments */
        if (!aparse__nargs_can_coalesce(current_arg->nargs)) {
            fprintf(stderr, " ");
        }
    }
    if (current_arg->nargs == APARSE_NARGS_0_OR_1) {
        fprintf(stderr, "[%s]", metavar);
    } else if (current_arg->nargs == APARSE_NARGS_0_OR_MORE) {
        fprintf(stderr, "[%s ...]", metavar);
    } else if (current_arg->nargs == APARSE_NARGS_1_OR_MORE) {
        fprintf(stderr, "%s [%s ...]", metavar, metavar);
    } else {
        int j = (int)current_arg->nargs;
        for (j = 0; j < current_arg->nargs; j++) {
            if (j) {
                fprintf(stderr, " ");
            }
            fprintf(stderr, "%s", metavar);
        }
    }
    if (!current_arg->required) {
        fprintf(stderr, "]");
    }
}

MN_INTERNAL void aparse__error_print_usage(aparse_state* state)
{
    size_t                    i;
    const struct aparse__arg* current_arg;
    if (state->argc == 0) {
        fprintf(stderr, "usage:");
    } else {
        fprintf(stderr, "usage: %s", state->argv[0]);
    }
    /* Print optional short options with no arguments first */
    aparse__error_print_usage_coalesce_short_args(state);
    /* Print other optional options */
    for (i = 0; i < state->args_size; i++) {
        current_arg = &state->args[i];
        /* Filter out positionals */
        if (current_arg->type == APARSE__ARG_TYPE_POSITIONAL) {
            continue;
        }
        /* Filter out required options */
        if (current_arg->required) {
            continue;
        }
        /* Filter out options we printed in coalesce */
        if (aparse__nargs_can_coalesce(current_arg->nargs)) {
            continue;
        }
        fprintf(stderr, " ");
        aparse__error_print_usage_arg(state, current_arg);
    }
    /* Print mandatory options */
    for (i = 0; i < state->args_size; i++) {
        current_arg = &state->args[i];
        /* Filter out positionals */
        if (current_arg->type == APARSE__ARG_TYPE_POSITIONAL) {
            continue;
        }
        /* Filter out optional options */
        if (!current_arg->required) {
            continue;
        }
        fprintf(stderr, " ");
        aparse__error_print_usage_arg(state, current_arg);
    }
    /* Print mandatory positionals */
    for (i = 0; i < state->args_size; i++) {
        current_arg = &state->args[i];
        /* Filter out optionals */
        if (current_arg->type == APARSE__ARG_TYPE_OPTIONAL) {
            continue;
        }
        /* Filter out optional positionals */
        if (!current_arg->required) {
            continue;
        }
        fprintf(stderr, " ");
        aparse__error_print_usage_arg(state, current_arg);
    }
    /* Print optional positionals */
    for (i = 0; i < state->args_size; i++) {
        current_arg = &state->args[i];
        /* Filter out optionals */
        if (current_arg->type == APARSE__ARG_TYPE_OPTIONAL) {
            continue;
        }
        /* Filter out mandatory positionals */
        if (current_arg->required) {
            continue;
        }
        fprintf(stderr, " ");
        aparse__error_print_usage_arg(state, current_arg);
    }
    fprintf(stderr, "\n");
}

MN_INTERNAL void aparse__error_begin(aparse_state* state)
{
    aparse__error_print_usage(state);
    if (state->argc == 0) {
        fprintf(stderr, "error: ");
    } else {
        fprintf(stderr, "%s: error: ", state->argv[0]);
    }
}

MN_INTERNAL void aparse__error_end(aparse_state* state)
{
    MN__UNUSED(state);
    fprintf(stderr, "\n");
}

MN_INTERNAL void aparse__error_arg_begin(aparse_state* state)
{
    MN__UNUSED(state);
}

MN_INTERNAL void aparse__error_arg_end(aparse_state* state)
{
    MN__UNUSED(state);
}
