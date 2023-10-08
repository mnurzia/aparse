#ifndef APARSE_H
#define APARSE_H

#include <stddef.h>

#define AP_ERR_NONE 0   /* no error */
#define AP_ERR_NOMEM -1 /* out of memory */
#define AP_ERR_PARSE -2 /* error when parsing */
#define AP_ERR_IO -3    /* error when printing */
#define AP_ERR_EXIT -4  /* exit main() immediately for -h and -v-like opts */

typedef struct ap ap;

/* customizable callbacks for ap instances */
typedef struct ap_ctxcb {
  void *uptr;                               /* user pointer */
  void *(*malloc)(void *, size_t);          /* get memory */
  void *(*realloc)(void *, void *, size_t); /* reallocate memory */
  void (*free)(void *, void *);             /* free memory */
  int (*out)(void *, const char *, size_t); /* print to stdout */
  int (*err)(void *, const char *, size_t); /* print to stderr */
} ap_ctxcb;

/* callback data passed to argument callbacks */
typedef struct ap_cb_data {
  const char *arg; /* pointer to argument (may be NULL)*/
  int arg_len;     /* strlen() of arg */
  int idx;         /* number of times callback has been called in a row */
  int more;        /* set this to 1 to have your callback called again */
  int destroy;     /* 1 if ap_custom_dtor() was called arg being destroyed */
  ap *parser;      /* pointer to the parser */
  void *reserved;
} ap_cb_data;

/* callback function for custom argument types
 * - uptr: user pointer
 * - pdata: pointer to callback data */
typedef int (*ap_cb)(void *uptr, ap_cb_data *pdata);

/* initialize parser
 * - progname: argv[0] */
ap *ap_init(const char *progname);

/* initialize parser (extended)
 * - out: set to the parser
 * - progname: argv[0]
 * - pctxcb: library callbacks (see `ap_ctxcb`)
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_NOMEM: out of memory
 *
 * If `pctxcb` is NULL, default values are used (stdlib malloc, fread, fwrite,
 * etc.)*/
int ap_init_full(ap **out, const char *progname, const ap_ctxcb *pctxcb);

/* destroy parser
 * - parser: the parser to destroy */
void ap_destroy(ap *parser);

/* set parser description for help text
 * - parser: the parser to set the description of
 * - description: the description to set */
void ap_description(ap *parser, const char *description);

/* set parser epilog for help text
 * - parser: the parser to set the epilog of
 * - epilog: the epilog to set */
void ap_epilog(ap *parser, const char *epilog);

/* begin positional argument
 * - parser: the parser to add a positional argument to
 * - metavar: the placeholder text for this argument (required)
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_NOMEM: out of memory */
int ap_pos(ap *parser, const char *metavar);

/* begin optional argument
 * - parser: the parser to add an optional argument to
 * - short_opt: short option char (like -o, may be '\0' for no short opt)
 * - long_opt: long option string (like --option, may be NULL for no long opt)
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_NOMEM: out of memory
 *
 * Either `short_opt` or `long_opt` may be '\0' or NULL, respectively, but both
 * may not. */
int ap_opt(ap *parser, char short_opt, const char *long_opt);

/* specify current argument as flag type argument
 * - parser: the parser to set the argument type of
 * - out: pointer to an integer that will be set to 1 when argument is specified
 *        in argv */
void ap_type_flag(ap *parser, int *out);

/* specify current argument as integer type argument
 * - parser: the parser to set the argument type of
 * - out: pointer to an integer that will be set to the integer value of the
 *        argument specified in argv */
void ap_type_int(ap *parser, int *out);

/* specify current argument as string type argument
 * - parser: the parser to set the argument type of
 * - out: pointer to a string that will be set to argument specified in argv */
void ap_type_str(ap *parser, const char **out);

/* specify current argument as enum type argument
 * - parser: the parser to set the argument type of
 * - out: pointer to an integer that will hold the index in `choices` of the
 *        argument specified in argv
 * - choices: NULL-terminated array of string choices for the argument
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_NOMEM: out of memory */
int ap_type_enum(ap *parser, int *out, const char **choices);

/* specify current argument as one that shows help text immediately
 * - parser: the parser to set the argument type of
 *
 * If this option is specified in `argv`, `ap_parse` will return AP_ERR_EXIT. */
void ap_type_help(ap *parser);

/* specify current argument as one that shows version text immediately
 * - parser: the parser to set the argument type of
 * - version: the version text to show
 *
 * If this option is specified in `argv`, `ap_parse` will return AP_ERR_EXIT. */
void ap_type_version(ap *parser, const char *version);

/* specify current argument as subparser
 * - parser: the parser to set the argument type of
 * - metavar: the metavar to set for the subparser
 * - out_idx: set to the index of the subparser that was selected for parsing */
void ap_type_sub(ap *parser, const char *metavar, int *out_idx);

/* add subparser selection to subparser argument
 * - parser: the parser that will have a new subparser added to its current
 *           subparser argument
 * - name: value specified in `argv` to trigger this subparser
 * - subpar: location of a newly-constructed `ap` object that represents the
 *           subparser itself
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_NOMEM: out of memory
 *
 * Setting `name` to the empty string "" is valid, but is really only useful for
 * triggering a subparser immediately on an option specification. */
int ap_sub_add(ap *parser, const char *name, ap **subpar);

/* specify current argument as a custom argument
 * - parser: the parser to set the argument type of
 * - callback: function called when this argument is specified in `argv`
 *             (see `ap_cb`)
 * - user: user pointer passed to `callback` */
void ap_type_custom(ap *parser, ap_cb callback, void *user);

/* signal that the current custom argument should be destroyed when the parser
 * is destroyed
 * - parser: the parser that will have its current argument's destructor status
 *           modified
 * - enable: 1 if the current argument's callback should be called with
 *           `ap_cb_data.destroy == 1` when `ap_destroy` is called, 0 if not */
void ap_custom_dtor(ap *parser, int enable);

/* specify help text for the current argument
 * - parser: the parser to set the help text of the current argument for
 * - help: the help text to set */
void ap_help(ap *parser, const char *help);

/* specify metavar for the current argument
 * - parser: the parser to set the metavar of the current argument for
 * - metavar: the metavar to set */
void ap_metavar(ap *parser, const char *metavar);

/* parse arguments
 * - parser: the parser to use for parsing `argc` and `argv`
 * - argc: the number of arguments in `argv`
 * - argv: an array of the arguments themselves
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_PARSE: parsing error, message was printed using `ap_ctxcb.err`
 * - AP_ERR_IO: I/O error when writing output
 * - AP_ERR_EXIT: argument specified exiting early (like -h or -v)
 *
 * Note that this function is not supposed to accept the program name (typically
 * argv[0] in `main`). So, a typical usage in `main` looks like:
 * `parser = ap_init(argv[0]);`
 * `ap_parse(parser, argc - 1, argv + 1);` */
int ap_parse(ap *parser, int argc, const char *const *argv);

/* show help text
 * - parser: the parser to show the help text of
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_IO: I/O error when writing output */
int ap_show_help(ap *parser);

/* show usage text
 * - parser: the parser to show the usage text of
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_IO: I/O error when writing output */
int ap_show_usage(ap *parser);

/* display an error with usage text
 * - parser: the parser to display the error for
 * - error_string: the error message
 * return:
 * - AP_ERR_NONE: no error
 * - AP_ERR_IO: I/O error when writing output */
int ap_error(ap *parser, const char *error_string);

/* display an error with additional context about an argument when in a callback
 * - parser: the parser to show the help text of
 * - error_string: the error message
 * return:
 * - AP_ERR_PARSE: no error (intended to be used as a direct return value)
 * - AP_ERR_IO: I/O error when writing output */
int ap_arg_error(ap_cb_data *cbd, const char *error_string);

#endif
