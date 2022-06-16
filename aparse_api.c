#include "aparse_internal.h"

MN_API aparse_error aparse_init(aparse_state* state) {
    state->state = (aparse__state*)MN_MALLOC(sizeof(aparse__state));
    if (state->state == MN_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    aparse__state_init(state->state);
    state->state->root = MN_MALLOC(sizeof(aparse__state_root));
        if (state->state->root == MN_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    state->state->root->out_buf_ptr = 0;
    state->state->root->prog_name = MN_NULL;
    state->state->root->prog_name_size = MN_NULL;
    state->state->is_root = 1;
    return APARSE_ERROR_NONE;
}

MN_API void aparse_destroy(aparse_state* state) {
    aparse__state_destroy(state->state);
    if (state->state != MN_NULL) {
        MN_FREE(state->state);
    }
}

MN_API void aparse_set_out_cb(aparse_state* state, aparse_out_cb out_cb, void* user) {
    aparse__state_set_out_cb(state->state, out_cb, user);
}

MN_API aparse_error aparse_add_opt(aparse_state* state, char short_opt, const char* long_opt) {
    aparse__state_check_before_add(state->state);
    return aparse__state_add_opt(state->state, short_opt, long_opt);
}

MN_API aparse_error aparse_add_pos(aparse_state* state, const char* name) {
    aparse__state_check_before_add(state->state);
    return aparse__state_add_pos(state->state, name);
}

MN_API aparse_error aparse_add_sub(aparse_state* state) {
    aparse__state_check_before_add(state->state);
    return aparse__state_add_sub(state->state);
}

MN_API void aparse_arg_help(aparse_state* state, const char* help_text) {
    aparse__state_check_before_modify(state->state);
    state->state->tail->help = help_text;
    if (help_text != MN_NULL) {
        state->state->tail->help_size = mn__slen(help_text);
    }
}

MN_API void aparse_arg_metavar(aparse_state* state, const char* metavar) {
    aparse__state_check_before_modify(state->state);
    state->state->tail->metavar = metavar;
    if (metavar != MN_NULL) {
        state->state->tail->metavar_size = mn__slen(metavar);
    }
}

MN_API void aparse_arg_type_bool(aparse_state* state, int* out) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_bool_init(state->state->tail, out);
}

MN_API void aparse_arg_type_str(aparse_state* state, const char** out, mn_size* out_size) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_str_init(state->state->tail, out, out_size);
}

MN_API void aparse_arg_type_help(aparse_state* state) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_help_init(state->state->tail);
}

MN_API void aparse_arg_type_version(aparse_state* state) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_version_init(state->state->tail);
}

MN_API void aparse_arg_type_custom(aparse_state* state, aparse_custom_cb cb, void* user, aparse_nargs nargs) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_custom_init(state->state->tail, cb, user, nargs);
}

MN_API aparse_error aparse_parse(aparse_state* state, int argc, const char* const* argv) {
    aparse_error err = APARSE_ERROR_NONE;
    if (argc == 0) {
        return APARSE_ERROR_INVALID;
    } else {
        state->state->root->prog_name = argv[0];
        state->state->root->prog_name_size = mn__slen(state->state->root->prog_name);
        err = aparse__parse_argv(state->state, argc - 1, argv + 1);
        if (err == APARSE_ERROR_PARSE) {
            if ((err = aparse__state_flush(state->state))) {
                return err;
            }
            return APARSE_ERROR_PARSE;
        } else if (err == APARSE_ERROR_SHOULD_EXIT) {
            if ((err = aparse__state_flush(state->state))) {
                return err;
            }
            return APARSE_ERROR_SHOULD_EXIT;
        } else {
            return err;
        }
    }
}
