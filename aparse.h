#ifndef APARSE_H
#define APARSE_H

#include <stddef.h>

#define AP_ERR_NONE 0
#define AP_ERR_NOMEM -1
#define AP_ERR_PARSE -2
#define AP_ERR_IO -3
#define AP_ERR_EXIT -4

typedef struct ap ap;

typedef struct ap_ctxcb {
  void *uptr;
  void *(*malloc)(void *, size_t);
  void *(*realloc)(void *, void *, size_t);
  void (*free)(void *, void *);
  int (*out)(void *, const char *, size_t);
  int (*err)(void *, const char *, size_t);
} ap_ctxcb;

typedef struct ap_cb_data {
  const char *arg;
  int arg_len;
  int idx;
  int more;
  int destroy;
  ap *parser;
  void *reserved;
} ap_cb_data;

typedef int (*ap_cb)(void *uptr, ap_cb_data *pdata);

ap *ap_init(const char *progname);
int ap_init_full(ap **out, const char *progname, const ap_ctxcb *pctxcb);
void ap_destroy(ap *par);
void ap_description(ap *par, const char *description);
void ap_epilog(ap *par, const char *epilog);

int ap_pos(ap *par, const char *metavar);
int ap_opt(ap *par, char short_opt, const char *long_opt);

void ap_type_flag(ap *par, int *out);
void ap_type_int(ap *par, int *out);
void ap_type_str(ap *par, const char **out);
int ap_type_enum(ap *par, int *out, const char **choices);
void ap_type_help(ap *par);
void ap_type_version(ap *par, const char *version);

void ap_help(ap *par, const char *help);
void ap_metavar(ap *par, const char *metavar);

void ap_type_sub(ap *par, const char *metavar, int *out_idx);
int ap_sub_add(ap *par, const char *name, ap **subpar);

void ap_type_custom(ap *par, ap_cb callback, void *user);
void ap_custom_dtor(ap *par, int enable);

int ap_parse(ap *par, int argc, const char *const *argv);

int ap_show_help(ap *par);
int ap_show_usage(ap *par);

int ap_error(ap *par, const char *error_string);
int ap_arg_error(ap_cb_data *cbd, const char *error_string);

#endif
