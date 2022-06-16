#include "aparse_internal.h"

MN_INTERNAL void aparse__arg_init(aparse__arg* arg) {
    arg->type = 0;
    arg->help = MN_NULL;
    arg->metavar = MN_NULL;
    arg->callback = MN_NULL;
    arg->callback_data = MN_NULL;
    arg->callback_data_2.plain = MN_NULL;
    arg->nargs = 0;
    arg->required = 0;
    arg->was_specified = 0;
    arg->next = MN_NULL;
}

MN_INTERNAL void aparse__arg_destroy(aparse__arg* arg) {
    if (arg->destroy != MN_NULL) {
        arg->destroy(arg);
    }
}

MN_INTERNAL void aparse__arg_bool_destroy(aparse__arg* arg);
MN_INTERNAL aparse_error aparse__arg_bool_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size);

MN_INTERNAL void aparse__arg_bool_init(aparse__arg* arg, int* out) {
    arg->nargs = APARSE_NARGS_0_OR_1_EQ;
    arg->callback = aparse__arg_bool_cb;
    arg->callback_data = (void*)out;
    arg->destroy = aparse__arg_bool_destroy;
}

MN_INTERNAL void aparse__arg_bool_destroy(aparse__arg* arg) {
    MN__UNUSED(arg);
}

MN_INTERNAL aparse_error aparse__arg_bool_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size) {
    aparse_error err = APARSE_ERROR_NONE;
    int* out = (int*)arg->callback_data;
    MN__UNUSED(state);
    MN__UNUSED(sub_arg_idx);
    if (text == MN_NULL) {
        *out = 1;
        return APARSE_ERROR_NONE;
    } else if (text_size == 1 && *text == '0') {
        *out = 0;
        return APARSE_ERROR_NONE;
    } else if (text_size == 1 && *text == '1') {
        *out = 1;
        return APARSE_ERROR_NONE;
    } else {
        if ((err = aparse__error_begin_arg(state, arg))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, "invalid value for boolean flag: "))) {
            return err;
        }
        if ((err = aparse__error_quote(state, text, text_size))) {
            return err;
        }
        if ((err = aparse__state_out(state, '\n'))) {
            return err;
        }
        return APARSE_ERROR_PARSE;
    }
}

MN_INTERNAL void aparse__arg_str_destroy(aparse__arg* arg);
MN_INTERNAL aparse_error aparse__arg_str_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size);

MN_INTERNAL void aparse__arg_str_init(aparse__arg* arg, const char** out, mn_size* out_size) {
    MN_ASSERT(out != MN_NULL);
    arg->nargs = 1;
    arg->callback = aparse__arg_str_cb;
    arg->callback_data = (void*)out;
    arg->callback_data_2.plain = (void*)out_size;
    arg->destroy = aparse__arg_str_destroy;
}

MN_INTERNAL void aparse__arg_str_destroy(aparse__arg* arg) {
    MN__UNUSED(arg);
}

MN_INTERNAL aparse_error aparse__arg_str_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size) {
    const char** out = (const char**)arg->callback_data;
    mn_size* out_size = (mn_size*)arg->callback_data_2.plain;
    MN_ASSERT(text != MN_NULL);
    MN__UNUSED(state);
    MN__UNUSED(sub_arg_idx);
    *out = text;
    if (out_size) {
        *out_size = text_size;
    }
    return APARSE_ERROR_NONE;
}

MN_INTERNAL void aparse__arg_help_destroy(aparse__arg* arg);
MN_INTERNAL aparse_error aparse__arg_help_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size);

MN_INTERNAL void aparse__arg_help_init(aparse__arg* arg) {
    arg->nargs = 0;
    arg->callback = aparse__arg_help_cb;
    arg->destroy = aparse__arg_help_destroy;
}

MN_INTERNAL void aparse__arg_help_destroy(aparse__arg* arg) {
    MN__UNUSED(arg);
}

MN_INTERNAL aparse_error aparse__arg_help_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size) {
    aparse_error err = APARSE_ERROR_NONE;
    MN__UNUSED(arg);
    MN__UNUSED(sub_arg_idx);
    MN__UNUSED(text);
    MN__UNUSED(text_size);
    if ((err = aparse__error_usage(state))) {
        return err;
    }
    {
        int has_printed_header = 0;
        aparse__arg* cur = state->head;
        while (cur) {
            if (cur->type != APARSE__ARG_TYPE_POSITIONAL) {
                cur = cur->next;
                continue;
            }
            if (!has_printed_header) {
                if ((err = aparse__state_out_s(state, "\npositional arguments:\n"))) {
                    return err;
                }
                has_printed_header = 1;
            }
            if ((err = aparse__state_out_s(state, "  "))) {
                return err;
            }
            if (cur->metavar == MN_NULL) {
                if ((err = aparse__state_out_n(state, cur->contents.pos.name, cur->contents.pos.name_size))) {
                    return err;
                }
            } else {
                if ((err = aparse__state_out_n(state, cur->metavar, cur->metavar_size))) {
                    return err;
                }
            }
            if ((err = aparse__state_out(state, '\n'))) {
                return err;
            }
            if (cur->help != MN_NULL) {
                if ((err = aparse__state_out_s(state, "    "))) {
                    return err;
                }
                if ((err = aparse__state_out_n(state, cur->help, cur->help_size))) {
                    return err;
                }
                if ((err = aparse__state_out(state, '\n'))) {
                    return err;
                }
            }
            cur = cur->next;
        }
    }
    {
        int has_printed_header = 0;
        aparse__arg* cur = state->head;
        while (cur) {
            if (cur->type != APARSE__ARG_TYPE_OPTIONAL) {
                cur = cur->next;
                continue;
            }
            if (!has_printed_header) {
                if ((err = aparse__state_out_s(state, "\noptional arguments:\n"))) {
                    return err;
                }
                has_printed_header = 1;
            }
            if ((err = aparse__state_out_s(state, "  "))) {
                return err;
            }
            if (cur->contents.opt.short_opt != '\0') {
                if ((err = aparse__error_print_short_opt(state, cur))) {
                    return err;
                }
                if (cur->nargs != APARSE_NARGS_0_OR_1_EQ &&
                    cur->nargs != 0) {
                    if ((err = aparse__state_out(state, ' '))) {
                        return err;
                    }
                }
                if ((err = aparse__error_print_sub_args(state, cur))) {
                    return err;
                }
            }
            if (cur->contents.opt.long_opt != MN_NULL) {
                if (cur->contents.opt.short_opt != '\0') {
                    if ((err = aparse__state_out_s(state, ", "))) {
                        return err;
                    }
                }
                if ((err = aparse__error_print_long_opt(state, cur))) {
                    return err;
                }
                if (cur->nargs != APARSE_NARGS_0_OR_1_EQ &&
                    cur->nargs != 0) {
                    if ((err = aparse__state_out(state, ' '))) {
                        return err;
                    }
                }
                if ((err = aparse__error_print_sub_args(state, cur))) {
                    return err;
                }
            }
            if ((err = aparse__state_out(state, '\n'))) {
                return err;
            }
            if (cur->help != MN_NULL) {
                if ((err = aparse__state_out_s(state, "    "))) {
                    return err;
                }
                if ((err = aparse__state_out_n(state, cur->help, cur->help_size))) {
                    return err;
                }
                if ((err = aparse__state_out(state, '\n'))) {
                    return err;
                }
            }
            cur = cur->next;
        }
    }
    return APARSE_SHOULD_EXIT;
}

MN_INTERNAL void aparse__arg_version_destroy(aparse__arg* arg);
MN_INTERNAL aparse_error aparse__arg_version_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size);

MN_INTERNAL void aparse__arg_version_init(aparse__arg* arg) {
    arg->nargs = 0;
    arg->callback = aparse__arg_version_cb;
    arg->destroy = aparse__arg_version_destroy;
}

MN_INTERNAL void aparse__arg_version_destroy(aparse__arg* arg) {
    MN__UNUSED(arg);
}

MN_INTERNAL aparse_error aparse__arg_version_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size) {
    aparse_error err = APARSE_ERROR_NONE;
    MN__UNUSED(arg);
    MN__UNUSED(sub_arg_idx);
    MN__UNUSED(text);
    MN__UNUSED(text_size);
    /* TODO: print version */
    if ((err = aparse__state_out_s(state, "version\n"))) {
        return err;
    }
    return APARSE_SHOULD_EXIT;
}

MN_INTERNAL void aparse__arg_custom_destroy(aparse__arg* arg);
MN_INTERNAL aparse_error aparse__arg_custom_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size);

MN_INTERNAL void aparse__arg_custom_init(aparse__arg* arg, aparse_custom_cb cb, void* user, aparse_nargs nargs) {
    arg->nargs = nargs;
    arg->callback = aparse__arg_custom_cb;
    arg->callback_data = (void*)user;
    arg->callback_data_2.custom_cb = cb;
    arg->destroy = aparse__arg_custom_destroy;
}

MN_INTERNAL void aparse__arg_custom_destroy(aparse__arg* arg) {
    MN__UNUSED(arg);
}

MN_INTERNAL aparse_error aparse__arg_custom_cb(aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx, const char* text, mn_size text_size) {
    aparse_custom_cb cb = (aparse_custom_cb)arg->callback_data_2.custom_cb;
    aparse_state state_;
    state_.state = state;
    return cb(arg->callback_data, &state_, (int)sub_arg_idx, text, text_size);
}

MN_INTERNAL void aparse__arg_sub_destroy(aparse__arg* arg);

MN_INTERNAL void aparse__arg_sub_init(aparse__arg* arg) {
    arg->type = APARSE__ARG_TYPE_SUBCOMMAND;
    arg->contents.sub.head = MN_NULL;
    arg->contents.sub.tail = MN_NULL;
    arg->destroy = aparse__arg_sub_destroy;
}

MN_INTERNAL void aparse__arg_sub_destroy(aparse__arg* arg) {
    aparse__sub* sub = arg->contents.sub.head;
    MN_ASSERT(arg->type == APARSE__ARG_TYPE_SUBCOMMAND);
    while (sub) {
        aparse__sub* prev = sub;
        aparse__state_destroy(&prev->subparser);
        sub = prev->next;
        MN_FREE(prev);
    }
}
