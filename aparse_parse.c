#include "aparse_internal.h"

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

/* Returns NULL if the option does not match. */
const char* aparse__arg_match_long_opt(const struct aparse__arg* opt,
    const char* arg_without_dashes)
{
    mn_size a_pos = 0;
    const char* a_str = opt->contents.opt.long_opt;
    const char* b = arg_without_dashes;
    while (1) {
        if (a_pos == opt->contents.opt.long_opt_size) {
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

MN_API aparse_error aparse__parse_argv(aparse__state* state, int argc, const char* const* argv) {
    aparse_error err = APARSE_ERROR_NONE;
    int argc_idx = 0;
    aparse__arg* next_positional = state->head;
    mn_size arg_text_size;
    APARSE__NEXT_POSITIONAL(next_positional);
    aparse__state_reset(state);
    while (argc_idx < argc) {
        const char* arg_text = argv[argc_idx++];
        if (aparse__is_positional(arg_text)) {
            if (next_positional == MN_NULL) {
                if ((err = aparse__error_unrecognized_arg(state, arg_text))) {
                    return err;
                }
                return APARSE_ERROR_PARSE;
            }
            arg_text_size = mn__slen((const mn_char*)arg_text);
            if ((err = next_positional->callback(next_positional, state, 0, arg_text, arg_text_size))) {
                return err;
            }
            APARSE__NEXT_POSITIONAL(next_positional);
        } else {
            int is_long = 0;
            const char* arg_end;
            if (arg_text[0] == '-' && arg_text[1] != '-') {
                arg_end = arg_text + 1;
            } else {
                arg_end = arg_text + 2;
                is_long = 1;
            }
            do {
                aparse__arg* arg = state->head;
                int has_text_left = 0;
                if (!is_long) {
                    char short_opt = *(arg_end++);
                    while (1) {
                        if (arg == MN_NULL) {
                            break;
                        }
                        if (arg->type == APARSE__ARG_TYPE_OPTIONAL) {
                            if (arg->contents.opt.short_opt == short_opt) {
                                break;
                            }
                        }
                        arg = arg->next;
                    }
                    if (arg == MN_NULL) {
                        if ((err = aparse__error_unrecognized_arg(state, arg_text))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    }
                    has_text_left = *arg_end != '\0';
                } else {
                    while (1) {
                        if (arg == MN_NULL) {
                            break;
                        }
                        if (arg->type == APARSE__ARG_TYPE_OPTIONAL) {
                            if (arg->contents.opt.long_opt != MN_NULL) {
                                mn_size opt_pos = 0;
                                const char* opt_ptr = arg->contents.opt.long_opt;
                                const char* arg_ptr = arg_end;
                                int found = 0;
                                while (1) {
                                    if (opt_pos == arg->contents.opt.long_opt_size) {
                                        if (*arg_ptr != '\0' && *arg_ptr != '=') {
                                            break;
                                        } else {
                                            /* *b equals '\0' or '=' */
                                            arg_end = arg_ptr;
                                            found = 1;
                                            break;
                                        }
                                    }
                                    if (*arg_ptr == '\0' || opt_ptr[opt_pos] != *arg_ptr) {
                                        /* b ended first or a and b do not match */
                                        break;
                                    }
                                    opt_pos++;
                                    arg_ptr++;
                                }
                                if (found) {
                                    break;
                                }
                            }
                        }
                        arg = arg->next;
                    }
                    if (arg == MN_NULL) {
                        if ((err = aparse__error_unrecognized_arg(state, arg_text))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    }
                }
                if (*arg_end == '=') {
                    /* use equals as argument */
                    if (arg->nargs == 0
                        || arg->nargs > 1) {
                        if ((err = aparse__error_begin_arg(state, arg))) {
                            return err;
                        }
                        if ((err = aparse__state_out_s(state, "cannot parse '='\n"))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    } else  {
                        arg_end++;
                        if ((err = arg->callback(arg, state, 0, arg_end, mn__slen(arg_end)))) {
                            return err;
                        }
                    }
                    break;
                } else if (has_text_left) {
                    /* use rest of arg as argument */
                    if (arg->nargs > 1) {
                        if ((err = aparse__error_begin_arg(state, arg))) {
                            return err;
                        }
                        if ((err = aparse__state_out_s(state, "cannot parse '"))) {
                            return err;
                        }
                        if ((err = aparse__state_out_s(state, arg_end))) {
                            return err;
                        }
                        if ((err = aparse__state_out(state, '\n'))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    } else if (arg->nargs != APARSE_NARGS_0_OR_1_EQ &&
                        arg->nargs != 0) {
                        if ((err = arg->callback(arg, state, 0, arg_end, mn__slen(arg_end)))) {
                            return err;
                        }
                        break;
                    } else {
                        if ((err = arg->callback(arg, state, 0, MN_NULL, 0))) {
                            return err;
                        }
                        /* fallthrough, continue parsing short options */
                    }
                } else if (argc_idx == argc || !aparse__is_positional(argv[argc_idx])) {
                    if (arg->nargs == APARSE_NARGS_1_OR_MORE
                        || arg->nargs == 1
                        || arg->nargs > 1) {
                        if ((err = aparse__error_begin_arg(state, arg))) {
                            return err;
                        }
                        if ((err = aparse__state_out_s(state, "expected an argument\n"))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    } else if (arg->nargs == APARSE_NARGS_0_OR_1_EQ
                            || arg->nargs == 0) {
                        if ((err = arg->callback(arg, state, 0, MN_NULL, 0))) {
                            return err;
                        }
                        /* fallthrough */  
                    } else {
                        if ((err = arg->callback(arg, state, 0, MN_NULL, 0))) {
                            return err;
                        }
                    }
                    break;
                } else {
                    if (arg->nargs == APARSE_NARGS_0_OR_1
                        || arg->nargs == 1) {
                        arg_text = argv[argc_idx++];
                        arg_text_size = mn__slen(arg_text);
                        if ((err = arg->callback(arg, state, 0, arg_text, arg_text_size))) {
                            return err;
                        }
                    } else if (arg->nargs == APARSE_NARGS_0_OR_1_EQ
                            || arg->nargs == 0) {
                        if ((err = arg->callback(arg, state, 0, MN_NULL, 0))) {
                            return err;
                        }
                    } else {
                        mn_size sub_arg_idx = 0;
                        while (argc_idx < argc) {
                            arg_text = argv[argc_idx++];
                            arg_text_size = mn__slen(arg_text);
                            if ((err = arg->callback(arg, state, sub_arg_idx++, arg_text, arg_text_size))) {
                                return err;
                            }
                            if ((int)sub_arg_idx == arg->nargs) {
                                break;
                            }
                        }
                        if ((int)sub_arg_idx != arg->nargs) {
                            if ((err = aparse__error_begin_arg(state, arg))) {
                                return err;
                            }
                            if ((err = aparse__state_out_s(state, "expected an argument\n"))) {
                                return err;
                            }
                            return APARSE_ERROR_PARSE;
                        }
                    }
                    break;
                }
            } while (!is_long);
        }
    }
    return err;
}

#undef APARSE__NEXT_POSITIONAL
