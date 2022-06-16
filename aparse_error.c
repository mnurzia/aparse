#include "aparse_internal.h"

MN_INTERNAL aparse_error aparse__error_begin_progname(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__state_out_n(state, state->root->prog_name, state->root->prog_name_size))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, ": "))) {
        return err;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__error_begin(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__error_begin_progname(state))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, "error: "))) {
        return err;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__error_print_short_opt(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    MN_ASSERT(arg->contents.opt.short_opt);
    if ((err = aparse__state_out(state, '-'))) {
        return err;
    }
    if ((err = aparse__state_out(state, arg->contents.opt.short_opt))) {
        return err;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__error_print_long_opt(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    MN_ASSERT(arg->contents.opt.long_opt);
    if ((err = aparse__state_out_s(state, "--"))) {
        return err;
    }
    if ((err = aparse__state_out_n(state, arg->contents.opt.long_opt, arg->contents.opt.long_opt_size))) {
        return err;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__error_begin_opt(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    MN_ASSERT(arg->type == APARSE__ARG_TYPE_OPTIONAL);
    if ((err = aparse__error_begin(state))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, "option "))) {
        return err;
    }
    if (arg->contents.opt.short_opt != '\0') {
        if ((err = aparse__error_print_short_opt(state, arg))) {
            return err;
        }
    }
    if (arg->contents.opt.long_opt != MN_NULL) {
        if (arg->contents.opt.short_opt != '\0') {
            if ((err = aparse__state_out_s(state, ", "))) {
                return err;
            }
        }
        if ((err = aparse__error_print_long_opt(state, arg))) {
            return err;
        }
    }
    if ((err = aparse__state_out_s(state, ": "))) {
        return err;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__error_begin_pos(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    MN_ASSERT(arg->type == APARSE__ARG_TYPE_POSITIONAL);
    if ((err = aparse__error_begin(state))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, "argument "))) {
        return err;
    }
    if ((err = aparse__state_out_n(state, arg->contents.pos.name, arg->contents.pos.name_size))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, ": "))) {
        return err;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__error_begin_arg(aparse__state* state, const aparse__arg* arg) {
    if (arg->type == APARSE__ARG_TYPE_OPTIONAL) {
        return aparse__error_begin_opt(state, arg);
    } else {
        return aparse__error_begin_pos(state, arg);
    }
}

MN_INTERNAL aparse_error aparse__error_unrecognized_arg(aparse__state* state, const char* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__error_begin(state))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, "unrecognized argument: "))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, arg))) {
        return err;
    }
    if ((err = aparse__state_out(state, '\n'))) {
        return err;
    }
    return err;
}

MN_INTERNAL char aparse__hexdig(unsigned char c) {
    if (c < 10) {
        return '0' + (char)c;
    } else {
        return 'a' + ((char)c - 10);
    }
}

MN_INTERNAL aparse_error aparse__error_quote(aparse__state* state, const char* text, mn_size text_size) {
    aparse_error err = APARSE_ERROR_NONE;
    mn_size i;
    if ((err = aparse__state_out(state, '"'))) {
        return err;
    }
    for (i = 0; i < text_size; i++) {
        char c = text[i];
        if (c < ' ') {
            if ((err = aparse__state_out(state, '\\'))) {
                return err;
            }
            if ((err = aparse__state_out(state, 'x'))) {
                return err;
            }
            if ((err = aparse__state_out(state, aparse__hexdig((c >> 4) & 0xF)))) {
                return err;
            }
            if ((err = aparse__state_out(state, aparse__hexdig(c & 0xF)))) {
                return err;
            }
        } else {
            if ((err = aparse__state_out(state, c))) {
                return err;
            }
        }
    }
    if ((err = aparse__state_out(state, '"'))) {
        return err;
    }
    return err;
}

int aparse__error_can_coalesce_in_usage(const aparse__arg* arg) {
    if (arg->type != APARSE__ARG_TYPE_OPTIONAL) {
        return 0;
    }
    if (arg->required) {
        return 0;
    }
    if (arg->contents.opt.short_opt == '\0') {
        return 0;
    }
    if ((arg->nargs != APARSE_NARGS_0_OR_1_EQ) && (arg->nargs != 0)) {
        return 0;
    }
    return 1;
}

MN_INTERNAL aparse_error aparse__error_print_sub_args(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    const char* var;
    mn_size var_size;
    if (arg->metavar != MN_NULL) {
        var = arg->metavar;
        var_size = arg->metavar_size;
    } else if (arg->type == APARSE__ARG_TYPE_POSITIONAL) {
        var = arg->contents.pos.name;
        var_size = arg->contents.pos.name_size;
    } else {
        var = "ARG";
        var_size = 3;
    }
    if (arg->nargs == APARSE_NARGS_1_OR_MORE) {
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, " ["))) {
            return err;
        }
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, " ...]]"))) {
            return err;
        }
    } else if (arg->nargs == APARSE_NARGS_0_OR_MORE) {
        if ((err = aparse__state_out(state, '['))) {
            return err;
        }
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, " ["))) {
            return err;
        }
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, " ...]]"))) {
            return err;
        }
    } else if (arg->nargs == APARSE_NARGS_0_OR_1_EQ) {
        /* pass */
    } else if (arg->nargs == APARSE_NARGS_0_OR_1) {
        if ((err = aparse__state_out(state, '['))) {
            return err;
        }
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out(state, ']'))) {
            return err;
        }
    } else if (arg->nargs > 0) {
        int i;
        for (i = 0; i < arg->nargs; i++) {
            if (i) {
                if ((err = aparse__state_out(state, ' '))) {
                    return err;
                }
            }
            if ((err = aparse__state_out_n(state, var, var_size))) {
                return err;
            }
        }
    }
    return err;
}

MN_INTERNAL aparse_error aparse__error_usage(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    const aparse__arg* cur = state->head;
    int has_printed = 0;
    if ((err = aparse__state_out_s(state, "usage: "))) {
        return err;
    }
    if ((err = aparse__state_out_n(state, state->root->prog_name, state->root->prog_name_size))) {
        return err;
    }
    /* print coalesced args */
    while (cur) {
        if (aparse__error_can_coalesce_in_usage(cur)) {
            if (!has_printed) {
                if ((err = aparse__state_out_s(state, " [-"))) {
                    return err;
                }
                has_printed = 1;
            }
            if ((err = aparse__state_out(state, cur->contents.opt.short_opt))) {
                return err;
            }
        }
        cur = cur->next;
    }
    if (has_printed) {
        if ((err = aparse__state_out(state, ']'))) {
            return err;
        }
    }
    /* print other args */
    cur = state->head;
    while (cur) {
        if (!aparse__error_can_coalesce_in_usage(cur)) {
            if ((err = aparse__state_out(state, ' '))) {
                return err;
            }
            if (!cur->required) {
                if ((err = aparse__state_out(state, '['))) {
                    return err;
                }
            }
            if (cur->type == APARSE__ARG_TYPE_OPTIONAL) {
                if (cur->contents.opt.short_opt) {
                    if ((err = aparse__error_print_short_opt(state, cur))) {
                        return err;
                    }
                } else if (cur->contents.opt.long_opt) {
                    if ((err = aparse__error_print_long_opt(state, cur))) {
                        return err;
                    }
                }
                if (cur->nargs != APARSE_NARGS_0_OR_1_EQ &&
                    cur->nargs != 0) {
                    if ((err = aparse__state_out(state, ' '))) {
                        return err;
                    }
                }
            }
            if ((err = aparse__error_print_sub_args(state, cur))) {
                return err;
            }
            if (!cur->required) {
                if ((err = aparse__state_out(state, ']'))) {
                    return err;
                }
            }
        }
        cur = cur->next;
    }
    if ((err = aparse__state_out(state, '\n'))) {
        return err;
    }
    return err;
}

#if 0
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

#endif
