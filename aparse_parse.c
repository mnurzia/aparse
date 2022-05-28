#include "aparse_internal.h"

enum aparse__parse_state
{
    /* ground state, looking for either optionals or positionals */
    APARSE__PARSE_STATE_GND,
    /* variant of ground state where we might be looking for an argument to an
     * optional argument */
    APARSE__PARSE_STATE_MAYBEARG,
    /* we are definitely looking for an argument */
    APARSE__PARSE_STATE_ARG
};

/* Returns 1 if the given argument might require parsing another element in
 * `argv` and checking if it doesn't start with '-' to determine if it has a
 * sub-argument. */
MN_INTERNAL int aparse__nargs_might_have_arg(aparse_nargs nargs)
{
    if (nargs == APARSE_NARGS_0_OR_1) {
        return 1;
    }
    if (nargs == APARSE_NARGS_0_OR_MORE) {
        return 1;
    }
    return 0;
}

/* Returns 1 if the given argument requires at least 1 sub-argument. */
MN_INTERNAL int aparse__nargs_has_required_arg(aparse_nargs nargs)
{
    if (nargs > 0) {
        return 1;
    }
    if (nargs == APARSE_NARGS_1_OR_MORE) {
        return 1;
    }
    return 0;
}

/* Returns 1 if the given argument can have a sub-argument specified with the
 * equals notation ('=') */
MN_INTERNAL int aparse__nargs_can_use_equals(aparse_nargs nargs)
{
    if (nargs == 1) {
        return 1;
    }
    if (nargs == APARSE_NARGS_0_OR_1) {
        return 1;
    }
    if (nargs == APARSE_NARGS_0_OR_1_EQ) {
        return 1;
    }
    if (nargs == APARSE_NARGS_1_OR_MORE) {
        return 1;
    }
    return 0;
}

/* Returns 1 if the given argument can use concatenation notation, like in
 * ('-Osubarg'). */
MN_INTERNAL int aparse__nargs_can_use_concat(aparse_nargs nargs)
{
    if (nargs == 1) {
        return 1;
    }
    if (nargs == APARSE_NARGS_1_OR_MORE) {
        return 1;
    }
    return 0;
}

/* Returns 1 if the given argument can be coalesced into the short options
 * listing at the beginning of usage messages. */
MN_INTERNAL int aparse__nargs_can_coalesce(aparse_nargs nargs)
{
    if (nargs == 0) {
        return 1;
    }
    if (nargs == APARSE_NARGS_0_OR_1_EQ) {
        return 1;
    }
    return 0;
}

/* Returns 1 if the given argument needs a sub-argument at a given position. */
MN_INTERNAL int aparse__nargs_needs_arg_at(aparse_nargs nargs, int sub_arg_idx)
{
    /* 0_OR_X will never need an argument. */
    if (nargs == APARSE_NARGS_0_OR_1) {
        return 0;
    }
    if (nargs == APARSE_NARGS_0_OR_1_EQ) {
        return 0;
    }
    if (nargs == APARSE_NARGS_0_OR_MORE) {
        return 0;
    }
    /* 1_OR_MORE will need an argument when `sub_arg_idx == 0`. */
    if (nargs == APARSE_NARGS_1_OR_MORE) {
        if (sub_arg_idx == 0) {
            return 1;
        } else {
            return 0;
        }
    }
    /* Everything else needs an argument when `sub_arg_idx` is less than
     * `nargs`. */
    if (nargs > 1) {
        if (sub_arg_idx < nargs) {
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}

/* Returns NULL if the option does not match. */
const char* aparse__arg_match_long_opt(const struct aparse__arg_new* opt,
    const char* arg_without_dashes)
{
    mn_size a_pos = 0;
    const char* a_str = opt->contents.opt.long_opt;
    const char* b = arg_without_dashes;
    while (1) {
        if (a_pos = opt->contents.opt.long_opt_size) {
            if (*b != '\0' && *b != '=') {
                return NULL;
            } else {
                /* *b equals '\0' or '=' */
                return b;
            }
        }
        if (*b == '\0' || a_str[a_pos] != *b) {
            /* b ended first or a and b do not match */
            return NULL;
        }
        a_pos++;
        b++;
    }
    return NULL;
}

MN_API int aparse_parse(aparse_state* state, int argc, const char* argv[])
{
    /* Index in `argv`. */
    int argv_idx = 0;
    /* If we found an option but still need its argument, store the option here
     * before transitioning into `APARSE__PARSE_STATE_ARG`. */
    struct aparse__arg* found_opt = NULL;
    /* If `parse_state == APARSE__PARSE_STATE_ARG`, then `found_opt` must be
     * non-NULL */
    enum aparse__parse_state parse_state = APARSE__PARSE_STATE_GND;
    int                      sub_arg_idx;
    state->argc = argc;
    state->argv = argv;
    while (argv_idx < argc) {
        const char* current_arg = argv[argv_idx];
        if (parse_state == APARSE__PARSE_STATE_GND) {
            sub_arg_idx = 0;
            if (current_arg[0] == '\0') {
                /* Empty argument. */
                /* TODO: Handle like a positional. */
            } else if (current_arg[0] != '-') {
                /* Non-empty non-option argument. */
                /* TODO: Handle like a positional. */
            } else if (current_arg[1] == '\0') {
                /* The argument "-". */
                /* TODO: Handle like a positional. */
            } else if (current_arg[1] == '-') {
                if (current_arg[2] == '\0') {
                    /* The argument "--". */
                    /* TODO: Stop parsing. */
                } else {
                    /* Long option. */
                    struct aparse__arg* current_opt;
                    size_t              i;
                    const char*         opt_end;
                    for (i = 0; i < state->args_size; i++) {
                        current_opt = state->args + i;
                        opt_end     = aparse__arg_match_long_opt(current_opt,
                            &(current_arg[2]));
                        if (opt_end != NULL) {
                            /* Argument was a match! */
                            break;
                        }
                    }
                    if (opt_end == NULL) {
                        /* Couldn't match long option. */
                        aparse__error_begin(state);
                        fprintf(stderr, "unrecognized argument: --%s",
                            &current_arg[2]);
                        aparse__error_end(state);
                        return 1;
                    }
                    /* Found long option. */
                    if (*opt_end == '\0') {
                        /* No argument was specified. */
                        if (aparse__nargs_might_have_arg(current_opt->nargs)) {
                            /* We might be able to get an argument. */
                            found_opt   = current_opt;
                            parse_state = APARSE__PARSE_STATE_MAYBEARG;
                        } else if (aparse__nargs_has_required_arg(
                                       current_opt->nargs)) {
                            /* We need to get an argument. */
                            /* If the option requires an argument, advance
                             * state to ARG. */
                            found_opt   = current_opt;
                            parse_state = APARSE__PARSE_STATE_ARG;
                        } else {
                            /* If not, just call the callback with NULL. */
                            current_opt->callback(state, -1,
                                current_opt->callback_data, NULL);
                        }
                    } else {
                        /* An argument was specified (`*opt_end == '='`) */
                        if (aparse__nargs_can_use_equals(current_opt->nargs)) {
                            /* We might be able to get an argument -- and we
                             * got one. */
                            current_opt->callback(state, 
                                sub_arg_idx, current_opt->callback_data,
                                opt_end + 1);
                        } else {
                            /* We used '=' notation when we weren't supposed
                             * to. */
                            aparse__error_begin(state);
                            fprintf(stderr, "argument ");
                            aparse__error_print_opt_name(state, current_opt);
                            fprintf(stderr, ": ignored explicit argument '%s'",
                                opt_end + 1);
                            aparse__error_end(state);
                            return 1;
                        }
                    }
                }
            } else {
                /* Short option(s). */
                const char* current_chr = &current_arg[1];
                size_t      num_parsed  = 0;
                while (*current_chr) {
                    struct aparse__arg* current_opt = NULL;
                    size_t              i;
                    if (*current_chr == '\0') {
                        /* Somehow we got "-\0" for our option. */
                        aparse__error_begin(state);
                        fprintf(stderr, "'\\0' is an invalid short option");
                        aparse__error_end(state);
                        return 1;
                    }
                    for (i = 0; i < state->args_size; i++) {
                        if (state->args[i].type
                            == APARSE__ARG_TYPE_POSITIONAL) {
                            /* Filter out positionals */
                            continue;
                        }
                        if (*current_chr == state->args[i].short_opt) {
                            current_opt = &(state->args[i]);
                            break;
                        }
                    }
                    if (current_opt == NULL) {
                        /* Couldn't match short option. */
                        aparse__error_begin(state);
                        fprintf(stderr, "unrecognized argument: -%c",
                            *current_chr);
                        aparse__error_end(state);
                        return 1;
                    }
                    /* Advance to next character */
                    current_chr++;
                    if (*current_chr == '\0') {
                        /* End of options */
                        if (aparse__nargs_might_have_arg(current_opt->nargs)) {
                            /* We might be able to get an argument. */
                            found_opt   = current_opt;
                            parse_state = APARSE__PARSE_STATE_MAYBEARG;
                        } else if (aparse__nargs_has_required_arg(
                                       current_opt->nargs)) {
                            /* We need to get an argument. */
                            /* If the option requires an argument, advance
                             * state to ARG. */
                            found_opt   = current_opt;
                            parse_state = APARSE__PARSE_STATE_ARG;
                        } else {
                            /* If not, just call the callback with NULL. */
                            current_opt->callback(state,  -1,
                                current_opt->callback_data, NULL);
                        }
                        break;
                    } else if (*current_chr == '=') {
                        /* Argument specified */
                        /* This allows options like "-xarg" and "-x=arg",
                         * but not "-yx=arg" or "-xyz=arg" */
                        /* For example, "-x=arg" would pass "arg" to x,
                           but "-yx=arg" would pass "=arg" to x */
                        if (num_parsed == 0) {
                            /* We have not already parsed an option, so
                             * allow '=' */
                            /* Advance character to get past '=' */
                            current_chr++;
                            current_opt->callback(state,
                                sub_arg_idx, current_opt->callback_data,
                                current_chr);
                        } else {
                            /* We've parsed an option */
                            /* Give the option the '=' */
                            current_opt->callback(state,
                                sub_arg_idx, current_opt->callback_data,
                                current_chr);
                        }
                        break;
                    } else {
                        if (aparse__nargs_can_use_concat(current_opt->nargs)) {
                            /* If the option can use concatenated args, take
                             * the rest of the argument as the arg to pass to
                             * the option */
                            current_opt->callback(state,
                                sub_arg_idx, current_opt->callback_data,
                                current_chr);
                            break;
                        } else {
                            if (aparse__nargs_has_required_arg(
                                    current_opt->nargs)) {
                                /* We need a sub-argument, we can't use
                                 * concatenation, and we didn't get one. */
                                aparse__error_begin(state);
                                fprintf(stderr, "argument ");
                                aparse__error_print_opt_name(state,
                                    current_opt);
                                if (aparse__nargs_can_use_equals(
                                        current_opt->nargs)) {
                                    fprintf(stderr,
                                        ": expected new argument or '=', ");
                                } else {
                                    fprintf(stderr,
                                        ": expected new argument, ");
                                }
                                fprintf(stderr, "but found '%s'", current_chr);
                                aparse__error_end(state);
                                return 1;
                            } else {
                                /* The option is perfectly fine with being one
                                 * of a long string of options, like "-xyzwh".
                                 * Call it as such, and continue to look for
                                 * options. */
                                current_opt->callback(state, -1,
                                    current_opt->callback_data, NULL);
                                num_parsed++;
                            }
                        }
                    }
                }
            }
            argv_idx++;
        } else if (parse_state == APARSE__PARSE_STATE_MAYBEARG) {
            /* If we don't have an argument yet but might be able to grab one,
             * we are here. We test if the first character is '-' and if so, we
             * treat it like another option, otherwise, we treat it as the
             * first sub-argument. */
            /* Note that we don't actually increment `argv_idx` here. This
             * state simply exists to choose whether or not to parse the next
             * argument as a sub-argument or an option. */
            if (current_arg[0] == '-') {
                parse_state = APARSE__PARSE_STATE_GND;
            } else {
                parse_state = APARSE__PARSE_STATE_ARG;
            }
        } else if (parse_state == APARSE__PARSE_STATE_ARG) {
            /* By convention, *any* argument starting with a '-' is interpreted
             * as an option. This means that when explicitly looking for an
             * argument, we can say that if it starts with a '-' it is an
             * error. */
            if (current_arg[0] == '-') {
                /* Expected an option, but an argument starting with a '-'
                 * appeared. */
                if (aparse__nargs_needs_arg_at(found_opt->nargs,
                        sub_arg_idx)) {
                    /* We needed an argument, but didn't get one. */
                    aparse__error_begin(state);
                    fprintf(stderr, "argument ");
                    aparse__error_print_opt_name(state, found_opt);
                    fprintf(stderr, ": expected ");
                    if (sub_arg_idx == 0) {
                        if (found_opt->nargs == APARSE_NARGS_1_OR_MORE) {
                            fprintf(stderr, "1 or more arguments");
                        } else if (found_opt->nargs > 1) {
                            fprintf(stderr, "%i arguments", found_opt->nargs);
                        }
                    } else {
                        fprintf(stderr, "%i arguments, got %i",
                            found_opt->nargs, sub_arg_idx);
                    }
                    aparse__error_end(state);
                    return 1;
                } else {
                    parse_state = APARSE__PARSE_STATE_GND;
                }
            } else {
                found_opt->callback(state, sub_arg_idx,
                    found_opt->callback_data, current_arg);
                argv_idx++;
                sub_arg_idx++;
            }
        }
    }
    /* Catch required-argument errors. */
    if (parse_state == APARSE__PARSE_STATE_ARG) {
        aparse__error_begin(state);
        fprintf(stderr, "argument ");
        aparse__error_print_opt_name(state, found_opt);
        fprintf(stderr, ": expected an argument");
        aparse__error_end(state);
        return 1;
    }
    return 0;
}

/* accepts an lvalue */
#define APARSE__NEXT_POSITIONAL(n) \
    while ((n) != MN_NULL && (n)->type != APARSE__ARG_TYPE_POSITIONAL) { \
        (n) = (n)->next; \
    }

int aparse__is_positional(const char* arg_text) {
    return (arg_text[0] == '\0') /* empty string */
            || (arg_text[0] == '-' && arg_text[1] == '\0') /* just a dash */
            || (arg_text[0] == '-' && arg_text[1] == '-' && arg_text[2] == '\0') /* two dashes */
            || (arg_text[0] != '-'); /* all positionals */
}

MN_API aparse_error aparse_parse_argv(aparse__state_new* state, int argc, const char* const* argv) {
    aparse_error err = APARSE_ERROR_NONE;
    int argc_idx = 0;
    aparse__arg_new* next_positional = state->head;
    mn_size arg_text_size;
    APARSE__NEXT_POSITIONAL(next_positional);
    aparse__state_reset(state);
    while (argc_idx < argc) {
        const char* arg_text = argv[argc_idx++];
        if (aparse__is_positional(arg_text)) {
            if (next_positional == MN_NULL) {
                /* TODO: error: unrecognized argument */
                MN_ASSERT(0);
            }
            arg_text_size = mn__slen(arg_text);
            if ((err = next_positional->callback(state, next_positional, 0, arg_text, arg_text_size))) {
                return err;
            }
            APARSE__NEXT_POSITIONAL(next_positional);
        } else {
            int is_long = 0;
            const char* arg_end;
            if (arg_text[0] == '-') {
                arg_end = arg_text + 1;
            } else if (arg_text[0] == '-' && arg_text[1] == '-') {
                arg_end = arg_text + 2;
                is_long = 1;
            }
            do {
                aparse__arg_new* arg = state->head;
                int has_text_left = 0;
                if (!is_long) {
                    char short_opt = *(arg_end++);
                    aparse__arg_new* arg = state->head;
                    while (arg && arg->type != APARSE__ARG_TYPE_OPTIONAL && arg->contents.opt.short_opt != short_opt) {
                        arg = arg->next;
                    }
                    if (arg == MN_NULL) {
                        /* TODO: error: unrecognized short argument */
                        return APARSE_ERROR_PARSE;
                    }
                    has_text_left = *arg_end != '\0';
                } else {
                    const char* arg_end;
                    while (1) {
                        if (arg == MN_NULL) {
                            break;
                        }
                        if (arg->type == APARSE__ARG_TYPE_POSITIONAL) {
                            if (arg->contents.opt.long_opt != MN_NULL) {
                                arg_end = aparse__arg_match_long_opt(arg, arg_end);
                                if (arg_end != MN_NULL) {
                                    break;
                                }
                            }
                        }
                        arg = arg->next;
                    }
                    if (arg == MN_NULL) {
                        /* TODO: error: long option not found */
                        return APARSE_ERROR_PARSE;
                    }
                }
                if (*arg_end == '=') {
                    /* use equals as argument */
                    if (arg->nargs == 0
                        || arg->nargs > 1) {
                        /* TODO: error: cannot use = */
                    } else  {
                        arg_end++;
                        if ((err = arg->callback(state, arg, 0, arg_end, mn__slen(arg_end)))) {
                            return err;
                        }
                    }
                    break;
                } else if (has_text_left) {
                    /* use rest of arg as argument */
                    if (arg->nargs > 1) {
                        /* TODO: error: cannot use extended */
                        break;
                    } else if (arg->nargs != APARSE_NARGS_0_OR_1_EQ &&
                        arg->nargs != 0) {
                        if ((err = arg->callback(state, arg, 0, arg_end, mn__slen(arg_end)))) {
                            return err;
                        }
                        break;
                    } else {
                        if ((err = arg->callback(state, arg, 0, MN_NULL, 0))) {
                            return err;
                        }
                        /* fallthrough, continue parsing short options */
                    }
                } else if (argc_idx == argc || !aparse__is_positional(argv[argc_idx])) {
                    if (arg->nargs == APARSE_NARGS_1_OR_MORE
                        || arg->nargs == 1
                        || arg->nargs > 1) {
                        /* TODO: error: need argument(s) for arg */
                    } else if (arg->nargs == APARSE_NARGS_0_OR_1_EQ
                            || arg->nargs == 0) {
                        if ((err = arg->callback(state, arg, 0, MN_NULL, 0))) {
                            return err;
                        }
                        /* fallthrough */  
                    } else {
                        if ((err = arg->callback(state, arg, 0, MN_NULL, 0))) {
                            return err;
                        }
                    }
                    break;
                } else {
                    if (arg->nargs == APARSE_NARGS_0_OR_1
                        || arg->nargs == 1) {
                        arg_text = argv[argc_idx++];
                        arg_text_size = mn__slen(arg_text);
                        if ((err = arg->callback(state, arg, 0, arg_text, arg_text_size))) {
                            return err;
                        }
                    } else if (arg->nargs == APARSE_NARGS_0_OR_1_EQ
                            || arg->nargs == 0) {
                        if ((err = arg->callback(state, arg, 0, MN_NULL, 0))) {
                            return err;
                        }
                    } else {
                        mn_size sub_arg_idx = 0;
                        while (argc_idx < argc) {
                            arg_text = argv[argc_idx++];
                            arg_text_size = mn__slen(arg_text);
                            if ((err = arg->callback(state, arg, sub_arg_idx++, arg_text, arg_text_size))) {
                                return err;
                            }
                            if (sub_arg_idx == arg->nargs) {
                                break;
                            }
                        }
                        if (sub_arg_idx != arg->nargs) {
                            /* TODO: error: not enough arguments specified */
                        }
                    }
                    break;
                }
            } while (!is_long);
        }
    }
}

#undef APARSE__NEXT_POSITIONAL
