#include <stdio.h>

#include "_cpack/internal.h"

#include "aparse_api.h"

typedef struct aparse__arg aparse__arg;

typedef struct aparse__arg_opt {
    char short_opt;
    const char* long_opt;
    mn_size long_opt_size;
} aparse__arg_opt;

typedef struct aparse__sub aparse__sub;

typedef struct aparse__arg_sub {
    aparse__sub* head;
    aparse__sub* tail;
} aparse__arg_sub;

typedef struct aparse__arg_pos {
    const char* name;
    mn_size name_size;
} aparse__arg_pos;

typedef union aparse__arg_contents {
    aparse__arg_opt opt;
    aparse__arg_sub sub;
    aparse__arg_pos pos;
} aparse__arg_contents;

enum aparse__arg_type {
    /* Optional argument (-o, --o) */
    APARSE__ARG_TYPE_OPTIONAL,
    /* Positional argument */
    APARSE__ARG_TYPE_POSITIONAL,
    /* Subcommand argument */
    APARSE__ARG_TYPE_SUBCOMMAND
};

typedef aparse_error (*aparse__arg_parse_cb)(
  aparse__arg* arg, aparse__state* state, mn_size sub_arg_idx,
  const char* text, mn_size text_size);
typedef void (*aparse__arg_destroy_cb)(aparse__arg* arg);

typedef union aparse__arg_callback_data_2 {
    void* plain;
    aparse_custom_cb custom_cb;
    int int_val;
} aparse__arg_callback_data_2;

struct aparse__arg {
    enum aparse__arg_type type;
    aparse__arg_contents contents;
    const char* help;
    mn_size help_size;
    const char* metavar;
    mn_size metavar_size;
    aparse_nargs nargs;
    int required;
    int was_specified;
    aparse__arg* next;
    aparse__arg_parse_cb callback;
    aparse__arg_destroy_cb destroy;
    void* callback_data;
    aparse__arg_callback_data_2 callback_data_2;
};

#define APARSE__STATE_OUT_BUF_SIZE 128

typedef struct aparse__state_root {
    char out_buf[APARSE__STATE_OUT_BUF_SIZE];
    mn_size out_buf_ptr;
    const char* prog_name;
    mn_size prog_name_size;
} aparse__state_root;

struct aparse__state {
    aparse__arg* head;
    aparse__arg* tail;
    const char* help;
    mn_size help_size;
    aparse_out_cb out_cb;
    void* user;
    aparse__state_root* root;
    int is_root;
};

struct aparse__sub {
    const char* name;
    mn_size name_size;
    aparse__state subparser;
    aparse__sub* next;
};

MN_INTERNAL void aparse__arg_init(aparse__arg* arg);
MN_INTERNAL void aparse__arg_destroy(aparse__arg* arg);
#if 0
MN_INTERNAL void aparse__state_init_from(aparse__state* state, aparse__state* other);
#endif
MN_INTERNAL void aparse__state_init(aparse__state* state);
MN_INTERNAL void aparse__state_destroy(aparse__state* state);
MN_INTERNAL void aparse__state_set_out_cb(
  aparse__state* state, aparse_out_cb out_cb, void* user);
MN_INTERNAL void aparse__state_reset(aparse__state* state);
MN_INTERNAL aparse_error aparse__state_add_opt(
  aparse__state* state, char short_opt, const char* long_opt);
MN_INTERNAL aparse_error
aparse__state_add_pos(aparse__state* state, const char* name);
MN_INTERNAL aparse_error aparse__state_add_sub(aparse__state* state);

MN_INTERNAL void aparse__state_check_before_add(aparse__state* state);
MN_INTERNAL void aparse__state_check_before_modify(aparse__state* state);
MN_INTERNAL void aparse__state_check_before_set_type(aparse__state* state);
MN_INTERNAL aparse_error aparse__state_flush(aparse__state* state);
MN_INTERNAL aparse_error aparse__state_out(aparse__state* state, char out);
MN_INTERNAL aparse_error
aparse__state_out_s(aparse__state* state, const char* s);
MN_INTERNAL aparse_error
aparse__state_out_n(aparse__state* state, const char* s, mn_size n);

MN_INTERNAL void aparse__arg_bool_init(aparse__arg* arg, int* out);
MN_INTERNAL void
aparse__arg_store_int_init(aparse__arg* arg, int val, int* out);
MN_INTERNAL void aparse__arg_int_init(aparse__arg* arg, int* out);
MN_INTERNAL void
aparse__arg_str_init(aparse__arg* arg, const char** out, mn_size* out_size);
MN_INTERNAL void aparse__arg_help_init(aparse__arg* arg);
MN_INTERNAL void aparse__arg_version_init(aparse__arg* arg);
MN_INTERNAL void aparse__arg_custom_init(
  aparse__arg* arg, aparse_custom_cb cb, void* user, aparse_nargs nargs);
MN_INTERNAL void aparse__arg_sub_init(aparse__arg* arg);

MN_API aparse_error
aparse__parse_argv(aparse__state* state, int argc, const char* const* argv);

MN_INTERNAL aparse_error aparse__error_begin(aparse__state* state);
MN_INTERNAL aparse_error
aparse__error_begin_arg(aparse__state* state, const aparse__arg* arg);
MN_INTERNAL aparse_error
aparse__error_unrecognized_arg(aparse__state* state, const char* arg);
MN_INTERNAL aparse_error
aparse__error_unrecognized_short_arg(aparse__state* state, char short_opt);
MN_INTERNAL aparse_error
aparse__error_quote(aparse__state* state, const char* text, mn_size text_size);
MN_INTERNAL aparse_error aparse__error_usage(aparse__state* state);
MN_INTERNAL aparse_error
aparse__error_print_short_opt(aparse__state* state, const aparse__arg* arg);
MN_INTERNAL aparse_error
aparse__error_print_long_opt(aparse__state* state, const aparse__arg* arg);
MN_INTERNAL aparse_error
aparse__error_print_sub_args(aparse__state* state, const aparse__arg* arg);
