#include "aparse_internal.h"

#include <stdio.h>

MN_INTERNAL aparse_error aparse__state_default_out_cb(void* user, const char* buf, mn_size buf_size) {
    MN__UNUSED(user);
    if (fwrite(buf, buf_size, 1, stdout) != 1) {
        return APARSE_ERROR_OUT;
    }
    return APARSE_ERROR_NONE;
}

MN_INTERNAL void aparse__state_init(aparse__state* state) {
    state->head = MN_NULL;
    state->tail = MN_NULL;
    state->help = MN_NULL;
    state->help_size = 0;
    state->out_cb = aparse__state_default_out_cb;
    state->user = MN_NULL;
    state->root = MN_NULL;
    state->is_root = 0;
}

#if 0

MN_INTERNAL void aparse__state_init_from(aparse__state* state, aparse__state* other) {
    aparse__state_init(state);
    state->out_cb = other->out_cb;
    state->user = other->user;
    state->root = other->root;
}

#endif

MN_INTERNAL void aparse__state_destroy(aparse__state* state) {
    aparse__arg* arg = state->head;
    while (arg) {
        aparse__arg* prev = arg;
        arg = arg->next;
        aparse__arg_destroy(prev);
        MN_FREE(prev);
    }
    if (state->is_root) {
        if (state->root != MN_NULL) {
            MN_FREE(state->root);
        }
    }
}

MN_INTERNAL void aparse__state_set_out_cb(aparse__state* state, aparse_out_cb out_cb, void* user) {
    state->out_cb = out_cb;
    state->user = user;
}

MN_INTERNAL void aparse__state_reset(aparse__state* state) {
    aparse__arg* cur = state->head;
    while (cur) {
        cur->was_specified = 0;
        if (cur->type == APARSE__ARG_TYPE_SUBCOMMAND) {
            aparse__sub* sub = cur->contents.sub.head;
            while (sub) {
                aparse__state_reset(&sub->subparser);
                sub = sub->next;
            }
        }
        cur = cur->next;
    }
}

MN_INTERNAL aparse_error aparse__state_arg(aparse__state* state) {
    aparse__arg* arg = (aparse__arg*)MN_MALLOC(sizeof(aparse__arg));
    if (arg == MN_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    aparse__arg_init(arg);
    if (state->head == MN_NULL) {
        state->head = arg;
        state->tail = arg;
    } else {
        state->tail->next = arg;
        state->tail = arg;
    }
    return APARSE_ERROR_NONE;
}

MN_INTERNAL void aparse__state_check_before_add(aparse__state* state) {
    /* for release builds */
    MN__UNUSED(state);

    /* If this fails, you forgot to specifiy a type for the previous argument. */
    MN_ASSERT(MN__IMPLIES(state->tail != MN_NULL, state->tail->callback != MN_NULL));
}

MN_INTERNAL void aparse__state_check_before_modify(aparse__state* state) {
    /* for release builds */
    MN__UNUSED(state);

    /* If this fails, you forgot to call add_opt() or add_pos(). */
    MN_ASSERT(state->tail != MN_NULL);
}

MN_INTERNAL void aparse__state_check_before_set_type(aparse__state* state) {
    /* for release builds */
    MN__UNUSED(state);

    /* If this fails, you forgot to call add_opt() or add_pos(). */
    MN_ASSERT(state->tail != MN_NULL);

    /* If this fails, you are trying to set the argument type of a subcommand. */
    MN_ASSERT(state->tail->type != APARSE__ARG_TYPE_SUBCOMMAND);

    /* If this fails, you called arg_xxx() twice. */
    MN_ASSERT(state->tail->callback == MN_NULL);
}

MN_INTERNAL aparse_error aparse__state_add_opt(aparse__state* state, char short_opt, const char* long_opt) {
    aparse_error err = APARSE_ERROR_NONE;
    /* If either of these fail, you specified both short_opt and long_opt as
     * NULL. For a positional argument, use aparse__add_pos. */
    MN_ASSERT(MN__IMPLIES(short_opt == '\0', long_opt != MN_NULL));
    MN_ASSERT(MN__IMPLIES(long_opt == MN_NULL, short_opt != '\0'));
    if ((err = aparse__state_arg(state))) {
        return err;
    }
    state->tail->type = APARSE__ARG_TYPE_OPTIONAL;
    state->tail->contents.opt.short_opt = short_opt;
    state->tail->contents.opt.long_opt = long_opt;
    if (long_opt != MN_NULL) {
        state->tail->contents.opt.long_opt_size = mn__slen(long_opt);
    } else {
        state->tail->contents.opt.long_opt_size = 0;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__state_add_pos(aparse__state* state, const char* name) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__state_arg(state))) {
        return err;
    }
    state->tail->type = APARSE__ARG_TYPE_POSITIONAL;
    state->tail->contents.pos.name = name;
    state->tail->contents.pos.name_size = mn__slen(name);
    return err;
}

MN_INTERNAL aparse_error aparse__state_add_sub(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__state_arg(state))) {
        return err;
    }
    aparse__arg_sub_init(state->tail);
    return err;
}

#if 0
MN_INTERNAL aparse_error aparse__state_sub_add_cmd(aparse__state* state, const char* name, aparse__state** subcmd) {
    aparse__sub* sub = (aparse__sub*)MN_MALLOC(sizeof(aparse__sub));
    MN_ASSERT(state->tail->type == APARSE__ARG_TYPE_SUBCOMMAND);
    MN_ASSERT(name != MN_NULL);
    if (sub == MN_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    sub->name = name;
    sub->name_size = mn__slen(name);
    aparse__state_init_from(&sub->subparser, state);
    sub->next = MN_NULL;
    if (state->tail->contents.sub.head == MN_NULL) {
        state->tail->contents.sub.head = sub;
        state->tail->contents.sub.tail = sub;
    } else {
        state->tail->contents.sub.tail->next = sub;
        state->tail->contents.sub.tail = sub;
    }
    *subcmd = &sub->subparser;
    return 0;
}

#endif


MN_INTERNAL aparse_error aparse__state_flush(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if (state->root->out_buf_ptr) {
        if ((err = state->out_cb(state->user, state->root->out_buf, state->root->out_buf_ptr))) {
            return err;
        }
        state->root->out_buf_ptr = 0;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__state_out(aparse__state* state, char out) {
    aparse_error err = APARSE_ERROR_NONE;
    if (state->root->out_buf_ptr == APARSE__STATE_OUT_BUF_SIZE) {
        if ((err = aparse__state_flush(state))) {
            return err;
        }
    }
    state->root->out_buf[state->root->out_buf_ptr++] = out;
    return err;
}

MN_INTERNAL aparse_error aparse__state_out_s(aparse__state* state, const char* s) {
    aparse_error err = APARSE_ERROR_NONE;
    while (*s) {
        if ((err = aparse__state_out(state, *s))) {
            return err;
        }
        s++;
    }
    return err;
}

MN_INTERNAL aparse_error aparse__state_out_n(aparse__state* state, const char* s, mn_size n) {
    aparse_error err = APARSE_ERROR_NONE;
    mn_size i;
    for (i = 0; i < n; i++) {
        if ((err = aparse__state_out(state, s[i]))) {
            return err;
        }
    }
    return err;
}
