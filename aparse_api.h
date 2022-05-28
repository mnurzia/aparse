#include <stddef.h>

#include "_cpack/api.h"

#define APARSE_ERROR_NONE 0
#define APARSE_ERROR_NOMEM -1
#define APARSE_ERROR_PARSE -2

typedef int aparse_error;

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
 * | <0_OR_1>    | *   | *    | *   | *   | *    | *   | *     | *     |
 * | <0_OR_1_EQ> | *   | *    | *   |     |      |     | *     |       |
 * | <0_OR_MORE> | *   | *    | *   | *   | *    | *   | *     | *     |
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
    APARSE__ARG_TYPE_POSITIONAL,
    /* Subcommand argument */
    APARSE__ARG_TYPE_SUBCOMMAND
};

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
    /* Whether the argument is positional or an option */
    enum aparse__arg_type type;
    /* Either a constant number of sub-arguments are accepted or a special
     * value from `enum aparse__nargs` to control argument amount behavior */
    aparse_nargs nargs;
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

MN_API int aparse_add_opt(aparse_state* state, char short_opt, const char* long_opt);
MN_API int aparse_add_pos(aparse_state* state, const char* metavar);
MN_API int aparse_add_sub(aparse_state* state, const char* metavar);

MN_API void aparse_arg_desc(aparse_state* state, const char* desc);
MN_API void aparse_arg_nargs(aparse_state* state, aparse_nargs nargs);
MN_API void aparse_arg_metavar(aparse_state* state, const char* metavar);
MN_API int aparse_arg_bool(aparse_state* state, int* out);
MN_API int aparse_arg_int(aparse_state* state, int* out);

MN_API aparse_error aparse_parse_argv(aparse_state* state, int argc, const char* const* argv);

MN_API int aparse_sub_add_cmd(aparse_state* state, const char* name, aparse__state_new** subcmd);
