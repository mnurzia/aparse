#ifndef MN_AP_H
#define MN_AP_H

#include <stddef.h>

#define AP_ERR_NONE 0
#define AP_ERR_NOMEM -1
#define AP_ERR_PARSE -2
#define AP_ERR_IO -3

/* How much validation should the library do? */

/* Want:
 * - Pos/opt
 * - Aliases
 * - Subparsers */

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
} ap_cb_data;

typedef int (*ap_cb)(void *uptr, ap_cb_data *pdata);

ap *ap_init(const char *progname);
int ap_init_full(ap **out, const char *progname, const ap_ctxcb *pctxcb);
void ap_destroy(ap *par);

int ap_begin_pos(ap *par, const char *metavar);
int ap_begin_opt(ap *par, char short_opt, const char *long_opt);
int ap_begin_sub(ap *par, const char *metavar);

void ap_end(ap *par); /* can only fail assert */

void ap_help(ap *par, const char *help_text);

int ap_sub_add(ap *par, const char *name, ap **subpar);

void ap_type_flag(ap *par, int *out);
void ap_type_int(ap *par, int *out);
void ap_type_str(ap *par, const char **out);
void ap_type_str_n(ap *par, const char **out, size_t *out_sz);
void ap_type_enum(ap *par, int *out, const char **choices);

void ap_type_custom(ap *par, ap_cb callback, void *user);

int ap_parse(ap *par, int argc, const char *const *argv);

#endif
