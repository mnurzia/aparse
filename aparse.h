#ifndef MN_AP_H
#define MN_AP_H

#include <stddef.h>

/* How much validation should the library do? */

/* Want:
 * - Pos/opt
 * - Aliases
 * - Subparsers
 * - Mutex groups */

typedef enum arg_type { POS, OPT, MTX, SUB } arg_type;

typedef struct arg arg;

typedef union arg_data {
  char opt_short;
  arg *mtx_children;
} arg_data;

typedef struct ap_cb_data {
  char *arg;
  size_t arg_len;
  size_t idx;
} ap_cb_data;

typedef int (*ap_cb)(void *user, ap_cb_data *pdata);

struct arg {
  arg_type arg_type;
  const char *name; /* metavar {pos, sub} / long opt {opt} */
  arg_data data;
  void *user;
  ap_cb *cb;
  arg *next;
};

typedef struct ap ap;
typedef struct ap_parent ap_parent;

struct ap_parent {
  ap *par;
  ap_parent *next;
};

struct ap {
  arg *args;
  ap_parent *parents;
};

ap *ap_init(void);
/* int ap_init_full(ap **out, ...); */
void ap_destroy(ap *par);

int ap_begin_pos(ap *par, const char *metavar);
int ap_begin_opt(ap *par, char short_opt, const char *long_opt);
int ap_begin_mtx(ap *par, ap *subpar);
int ap_begin_sub(ap *par, const char *metavar);

void ap_end(void); /* can only fail assert */

void ap_help(ap *par, const char *help_text);

int ap_sub_add(ap *par, const char *name, ap *subpar);

void ap_type_flag(ap *par, int *out);
void ap_type_int(ap *par, int *out);
void ap_type_str(ap *par, const char **out);
void ap_type_str_n(ap *par, const char **out, size_t *out_sz);
void ap_type_enum(ap *par, int *out, const char **choices);

void ap_type_custom(ap *par, ap_cb *callback, void *user);

int ap_parse(ap *par, int argc, const char *const *argv);

#endif
