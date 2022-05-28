#include "aparse_internal.h"

MN_INTERNAL void aparse__arg_new_init(aparse__arg_new* arg) {
    arg->type = -1;
    arg->help = MN_NULL;
    arg->metavar = MN_NULL;
    arg->callback = MN_NULL;
    arg->callback_data = MN_NULL;
    arg->nargs = 0;
    arg->required = 0;
    arg->was_specified = 0;
    arg->next = MN_NULL;
}

MN_INTERNAL void aparse__arg_new_destroy(aparse__arg_new* arg) {
    if (arg->type == APARSE__ARG_TYPE_SUBCOMMAND) {
        aparse__sub* sub = arg->contents.sub.head;
        while (sub) {
            aparse__sub* prev = sub;
            aparse__state_new_destroy(&prev->subparser);
            sub = prev->next;
            MN_FREE(prev);
        }
    }
}

MN_INTERNAL void aparse__state_new_init(aparse__state_new* state) {
    state->head = MN_NULL;
    state->tail = MN_NULL;
    state->help = MN_NULL;
}

MN_INTERNAL void aparse__state_new_destroy(aparse__state_new* state) {
    aparse__arg_new* arg = state->head;
    while (arg) {
        aparse__arg_new* prev = arg;
        arg = arg->next;
        MN_FREE(prev);
    }
}

MN_INTERNAL aparse_error aparse__new_arg(aparse__state_new* state) {
    aparse__arg_new* arg = (aparse__arg_new*)MN_MALLOC(sizeof(aparse__arg_new));
    if (arg == MN_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    aparse__arg_new_init(arg);
    if (state->head == MN_NULL) {
        state->head = arg;
        state->tail = arg;
    } else {
        state->tail->next = arg;
        state->tail = arg;
    }
    return APARSE_ERROR_NONE;
}

MN_INTERNAL aparse_error aparse__add_opt(aparse__state_new* state, char short_opt, const char* long_opt) {
    aparse_error err = APARSE_ERROR_NONE;
    /* If either of these fail, you specified both short_opt and long_opt as
     * NULL. For a positional argument, use aparse__add_pos. */
    MN_ASSERT(MN__IMPLIES(short_opt == '\0', long_opt != MN_NULL));
    MN_ASSERT(MN__IMPLIES(long_opt != MN_NULL, short_opt == '\0'));
    if ((err = aparse__new_arg(state))) {
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

MN_INTERNAL aparse_error aparse__add_pos(aparse__state_new* state, const char* metavar) {
    aparse_error err = APARSE_ERROR_NONE;
    /* If this fails, you specified metavar as NULL. */
    MN_ASSERT(metavar != MN_NULL);
    if ((err = aparse__new_arg(state))) {
        return err;
    }
    state->tail->type = APARSE__ARG_TYPE_POSITIONAL;
    state->tail->metavar = metavar;
    state->tail->metavar_size = mn__slen(metavar);
    return err;
}

MN_INTERNAL aparse_error aparse__add_sub(aparse__state_new* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__new_arg(state))) {
        return err;
    }
    state->tail->type = APARSE__ARG_TYPE_SUBCOMMAND;
    state->tail->contents.sub.head = MN_NULL;
    state->tail->contents.sub.tail = MN_NULL;
    return err;
}

MN_INTERNAL aparse_error aparse__sub_add_cmd(aparse__state_new* state, const char* name, aparse__state_new** subcmd) {
    aparse__sub* sub = (aparse__sub*)MN_MALLOC(sizeof(aparse__sub));
    MN_ASSERT(state->tail->type == APARSE__ARG_TYPE_SUBCOMMAND);
    MN_ASSERT(name != MN_NULL);
    if (sub == MN_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    sub->name = name;
    sub->name_size = mn__slen(name);
    aparse__state_new_init(&sub->subparser);
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

MN_INTERNAL void aparse__reset(aparse__state_new* state) {
    aparse__arg_new* cur = state->head;
    while (cur) {
        cur->was_specified = 0;
        if (cur->type == APARSE__ARG_TYPE_SUBCOMMAND) {
            aparse__sub* sub = cur->contents.sub.head;
            while (sub) {
                aparse_reset(&sub->subparser);
                sub = sub->next;
            }
        }
        cur = cur->next;
    }
}
