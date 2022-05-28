#if !defined(APARSE_H)
#define APARSE_H

/* Set to 1 in order to define malloc(), free(), and realloc() replacements. */
#if defined(MN_USE_CUSTOM_ALLOCATOR)
#define MN_MALLOC my_malloc
#define MN_REALLOC my_realloc
#define MN_FREE my_free
#else
#include <stdlib.h>
#define MN_MALLOC malloc
#define MN_REALLOC realloc
#define MN_FREE free
#endif

/* util/exports */
/* Set to 1 in order to define all symbols with static linkage (local to the
 * including source file) as opposed to external linkage. */
#if !defined(MN_STATIC)
#define MN_STATIC 0
#endif

/* util/exports */
#if !defined(MN__SPLIT_BUILD)
#if MN_STATIC
#define MN_API static
#else
#define MN_API extern
#endif
#else
#define MN_API extern
#endif

/* aparse */
#include <stddef.h>
typedef struct aparse_state aparse_state;

struct aparse__arg;

typedef struct aparse__arg aparse_arg;

typedef int (*aparse__arg_cb)(aparse_state* state, int sub_arg_idx, void* data, 
const char* text);

/* Syntax validity table: */
/* Where `O` is the option in question, `s` is a subargument to `O`, `P` is
 * another option (could be the same option), and `o` is the long option form
 * of `O`. */
/* |  args       | -O  | -O=s | -OP | -Os | -O s | --o | --o=s | --o s |
 * | ----------- | --- | ---- | --- | --- | ---- | --- | ----- | ----- |
 * | 2+          |     |      |     |     | *    |     |       | *     |
 * | 1           |     | *    |     | *   | *    |     | *     | *     |
 * | 0           | *   |      | *   |     |      | *   |       |       |
 * | <0_OR_1>    | *   | *    | *   |     | *    | *   | *     | *     |
 * | <0_OR_1_EQ> | *   | *    | *   |     |      |     | *     |       |
 * | <0_OR_MORE> | *   | *    | *   |     | *    | *   | *     | *     |
 * | <1_OR_MORE> |     | *    |     | *   | *    |     | *     | *     | */
typedef enum aparse_nargs
{
    /* Parse either zero or 1 subarguments. */
    APARSE_NARGS_0_OR_1 = -1, /* Like regex '?' */
    /* Parse either zero or 1 subarguments, but only allow using '='. */
    APARSE_NARGS_0_OR_1_EQ = -2, /* Like regex '?' */
    /* Parse zero or more subarguments. */
    APARSE_NARGS_0_OR_MORE = -3, /* Like regex '*' */
    /* Parse one or more subarguments. */
    APARSE_NARGS_1_OR_MORE = -4 /* Like regex '+' */
} aparse_nargs;

enum aparse__arg_type
{
    /* Optional argument (-o, --o) */
    APARSE__ARG_TYPE_OPTIONAL,
    /* Positional argument */
    APARSE__ARG_TYPE_POSITIONAL
};

struct aparse_state {
    /* Help/version text */
    const char* help;
    const char* version;
    /* List of arguments. Dynamically allocated if `APARSE_USE_MALLOC`,
     * otherwise it's a fixed-size array provided by `aparse_init_fixed` and is
     * unable to grow. `args_size` is how many arguments are stored,
     * `args_alloc` is the total size of the array in terms of `aparse__arg` */
    struct aparse__arg* args;
    size_t              args_size;
    size_t              args_alloc;
    /* The current argument under modification. `in_arg` is used to remember
     * not to increment `current_arg` initially. This prevents using another
     * function such as `aparse_arg_end` to increment it. */
    struct aparse__arg* current_arg;
    int                 in_arg;
    /* Program's argument count and argument vector */
    int          argc;
    const char** argv;
};

/* Initialize with fixed amount of options */
MN_API void aparse_init_fixed(aparse_state* state, struct aparse__arg* opts,
    size_t opts_alloc);

/* Initialize with varying amounts of options */
#if APARSE_USE_MALLOC
MN_API int aparse_init(aparse_state* state);
#endif

/* Destroy aparse instance */
MN_API void aparse_destroy(aparse_state* state);

/* Begin adding an option */
MN_API int aparse_add_opt(aparse_state* state, char short_opt, const char* long_opt);

MN_API void aparse_opt_desc(aparse_state* state, const char* desc);

MN_API void aparse_opt_nargs(aparse_state* state, aparse_nargs nargs);

MN_API void aparse_opt_metavar(aparse_state* state, const char* metavar);

MN_API int aparse_opt_bool(aparse_state* state, int* out);

MN_API int aparse_opt_int(aparse_state* state, int* out);

MN_API int aparse_parse(aparse_state* state, int argc, const char* argv[]);

/* util/exports */
#if !defined(MN__SPLIT_BUILD)
#define MN_INTERNAL static
#else
#define MN_INTERNAL extern
#endif

/* util/unused */
#define MN__UNUSED(x) ((void)(x))

/* aparse */
#include <stdio.h>
struct aparse__arg {
    /* Short option name (a single character) */
    char short_opt;
    /* Long option name (a string) */
    const char* long_opt;
    /* Description */
    const char* description;
    /* Variable for sub-arguments documentation */
    const char* metavar;
    /* Callback and associated data */
    aparse__arg_cb callback;
    void*          callback_data;
    /* Whether or not the argument is positional or an option */
    enum aparse__arg_type type;
    /* Either a constant number of sub-arguments are accepted or a special
     * value from `enum aparse__nargs` to control argument amount behavior */
    aparse_nargs nargs;
    /* Whether or not this argument is required */
    int required;
};

void aparse__arg_init(struct aparse__arg* opt, char short_opt,
    const char* long_opt);

int aparse__nargs_can_coalesce(aparse_nargs nargs);

void aparse__error_print_opt_name(aparse_state* state,
    const struct aparse__arg*                   opt);

void aparse__error_print_usage_coalesce_short_args(aparse_state* state);

void aparse__error_print_usage_arg(aparse_state* state,
    const struct aparse__arg*                    current_arg);

void aparse__error_print_usage(aparse_state* state);

void aparse__error_begin(aparse_state* state);

void aparse__error_end(aparse_state* state);

void aparse__error_arg_begin(aparse_state* state);

void aparse__error_arg_end(aparse_state* state);

#if defined(APARSE_IMPLEMENTATION)
/* aparse */
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

/* aparse */
MN_INTERNAL int aparse__arg_cb_bool(aparse_state* state, int sub_arg_idx, void* data, const char* arg)
{
    MN__UNUSED(state);
    MN__UNUSED(sub_arg_idx);
    MN__UNUSED(arg);
    *((int*)data) = 1;
    return 0;
}

MN_API int aparse_opt_bool(aparse_state* state, int* out)
{
    state->current_arg->nargs = APARSE_NARGS_0_OR_1_EQ;
    state->current_arg->callback = aparse__arg_cb_bool;
    state->current_arg->callback_data = (void*)out;
    return 0;
}

/* aparse */
MN_INTERNAL int aparse__arg_cb_int(aparse_state* state, int sub_arg_idx, void* data, const char* arg)
{
    int target;
    (void)(sub_arg_idx);
    if (sscanf(arg, "%i", &target) != 1) {
        aparse__error_begin(state);
        aparse__error_arg_begin(state);
        fprintf(stderr, "invalid int value: '%s'", arg);
        aparse__error_arg_end(state);
        aparse__error_end(state);
        return 1;
    }
    *((int*)(data)) = target;
    return 0;
}

MN_API int aparse_opt_int(aparse_state* state, int* out)
{
    state->current_arg->nargs = 1;
    state->current_arg->callback = aparse__arg_cb_int;
    state->current_arg->callback_data = (void*)out;
    return 0;
}

/* aparse */
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

/* aparse */
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
const char* aparse__arg_match_long_opt(const struct aparse__arg* opt,
    const char* arg_without_dashes)
{
    const char* a = opt->long_opt;
    const char* b = arg_without_dashes;
    while (1) {
        if (*a == '\0') {
            if (*b != '\0' && *b != '=') {
                return NULL;
            } else {
                /* *b equals '\0' or '=' */
                return b;
            }
        }
        if (*b == '\0' || *a != *b) {
            /* b ended first or a and b do not match */
            return NULL;
        }
        a++;
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

/* aparse */
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

#endif
#endif /* APARSE_H */
