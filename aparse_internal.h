#include <stdio.h>

#include "_cpack/internal.h"

#include "aparse_api.h"

typedef struct aparse__arg_new aparse__arg_new;

typedef aparse_error (*aparse__arg_new_cb)(aparse_state* state, aparse__arg_new* arg, int sub_arg_idx, const char* text, mn_size text_size);

typedef struct aparse__arg_new_opt {
    char short_opt;
    const char* long_opt;
    mn_size long_opt_size;
} aparse__arg_new_opt;

typedef struct aparse__sub {
    const char* name;
    mn_size name_size;
    aparse_state subparser;
    aparse__sub* next;
} aparse__sub;

typedef struct aparse__arg_new_sub {
    aparse__sub* head;
    aparse__sub* tail;
} aparse__arg_new_sub;

typedef struct aparse__arg_new_contents {
    aparse__arg_new_opt opt;
    aparse__arg_new_sub sub;
} aparse__arg_new_contents;

struct aparse__arg_new {
    enum aparse__arg_type type;
    aparse__arg_new_contents contents;
    const char* help;
    mn_size help_size;
    const char* metavar;
    mn_size metavar_size;
    aparse__arg_new_cb callback;
    void* callback_data;
    aparse_nargs nargs;
    int required;
    int was_specified;
    aparse__arg_new* next;
};

typedef struct aparse__state_new {
    aparse__arg_new* head;
    aparse__arg_new* tail;
    const char* help;
    mn_size help_size;
} aparse__state_new;

MN_INTERNAL void aparse__arg_new_init(aparse__arg_new* arg);

MN_INTERNAL void aparse__state_new_init(aparse__state_new* state);

MN_INTERNAL void aparse__state_new_destroy(aparse__state_new* state);

MN_INTERNAL void aparse__state_reset(aparse__state_new* state);

MN_INTERNAL void aparse__arg_init(struct aparse__arg* opt, char short_opt,
    const char* long_opt);

MN_INTERNAL int aparse__nargs_can_coalesce(aparse_nargs nargs);

MN_INTERNAL void aparse__error_print_opt_name(aparse_state* state,
    const struct aparse__arg*                   opt);

MN_INTERNAL void aparse__error_print_usage_coalesce_short_args(aparse_state* state);

MN_INTERNAL void aparse__error_print_usage_arg(aparse_state* state,
    const struct aparse__arg*                    current_arg);

MN_INTERNAL void aparse__error_print_usage(aparse_state* state);

MN_INTERNAL void aparse__error_begin(aparse_state* state);

MN_INTERNAL void aparse__error_end(aparse_state* state);

MN_INTERNAL void aparse__error_arg_begin(aparse_state* state);

MN_INTERNAL void aparse__error_arg_end(aparse_state* state);
