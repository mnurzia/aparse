#include "aparse.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* different types of arguments */
typedef enum ap_arg_type {
  AP_ARG_TYPE_POS, /* positional argument */
  AP_ARG_TYPE_OPT, /* optional argument */
  AP_ARG_TYPE_SUB  /* subparser */
} ap_arg_type;

typedef struct ap_arg ap_arg;

struct ap_arg {
  ap_arg_type arg_type; /* argument type */
  const char *name;     /* metavar {pos, sub} / long opt {opt} */
  char opt_short;       /* if short option, option character */
  void *user;           /* user pointer */
  ap_cb cb;             /* callback */
  ap_arg *next;         /* next argument in list */
};

/* parent linked list used in argument search order */
typedef struct ap_parent ap_parent;
struct ap_parent {
  ap *par;
  ap_parent *next;
};

/* subparser linked list used in subparser search order */
typedef struct ap_sub ap_sub;
struct ap_sub {
  const char *identifier; /* command name */
  ap *par;
  ap_sub *next;
};

/* argument parser */
struct ap {
  const ap_ctxcb *ctxcb; /* context callbacks (replicated in subparsers) */
  const char *progname;  /* argv[0] */
  ap_arg *args;          /* argument list */
  ap_arg *args_tail;     /* end of argument list */
  ap_arg *current;       /* current arg under modification */
  ap_parent *parents;    /* parent list for argument search order */
};

void *ap_malloc(void *uptr, size_t n) {
  (void)(uptr);
  return malloc(n);
}

void ap_free(void *uptr, void *ptr) {
  (void)(uptr);
  free(ptr);
}

void *ap_realloc(void *uptr, void *ptr, size_t n) {
  (void)uptr;
  return realloc(ptr, n);
}

int ap_out(void *uptr, const char *text, size_t n) {
  (void)uptr;
  return fwrite(text, 1, n, stdout) < n ? AP_ERR_IO : AP_ERR_NONE;
}

int ap_err(void *uptr, const char *text, size_t n) {
  (void)uptr;
  return fwrite(text, 1, n, stderr) < n ? AP_ERR_IO : AP_ERR_NONE;
}

static const ap_ctxcb ap_default_ctxcb = {NULL,    ap_malloc, ap_realloc,
                                          ap_free, ap_out,    ap_err};

ap *ap_init(const char *progname) {
  ap *out;
  return (ap_init_full(&out, progname, NULL) == AP_ERR_NONE) ? out : NULL;
}

int ap_init_full(ap **out, const char *progname, const ap_ctxcb *pctxcb) {
  ap *par;
  pctxcb = pctxcb ? pctxcb : &ap_default_ctxcb;
  par = pctxcb->malloc(pctxcb->uptr, sizeof(ap));
  if (!par)
    return AP_ERR_NOMEM;
  par->ctxcb = pctxcb;
  par->progname = progname;
  par->args = NULL;
  par->args_tail = NULL;
  par->current = NULL;
  par->parents = NULL;
  *out = par;
  return AP_ERR_NONE;
}

void ap_destroy(ap *par) {
  while (par->args) {
    ap_arg *prev = par->args;
    if (prev->arg_type == AP_ARG_TYPE_SUB) {
      ap_sub *sub = (ap_sub *)prev->user;
      while (sub) {
        ap_sub *prev_sub = sub;
        ap_destroy(prev_sub->par);
        sub = sub->next;
        par->ctxcb->free(par->ctxcb->uptr, prev_sub);
      }
    }
    par->args = par->args->next;
    par->ctxcb->free(par->ctxcb->uptr, prev);
  }
  par->ctxcb->free(par->ctxcb->uptr, par);
}

int ap_begin(ap *par) {
  ap_arg *next = (ap_arg *)par->ctxcb->malloc(par->ctxcb->uptr, sizeof(ap_arg));
  if (!next)
    return AP_ERR_NOMEM;
  memset(next, 0, sizeof(*next));
  if (!par->args) /* first argument, initialize list */
    par->args = next, par->args_tail = next;
  else /* next argument, link to end */
    par->args_tail->next = next, par->args_tail = next;
  par->current = next;
  return AP_ERR_NONE;
}

int ap_begin_pos(ap *par, const char *metavar) {
  int err = 0;
  if ((err = ap_begin(par)))
    return err;
  par->current->arg_type = AP_ARG_TYPE_POS;
  par->current->name = metavar;
  return 0;
}

int ap_begin_opt(ap *par, char short_opt, const char *long_opt) {
  int err = 0;
  if ((err = ap_begin(par)))
    return err;
  /* if this fails, you didn't specify a short or long opt */
  assert(short_opt != 0 || long_opt != NULL);
  par->current->arg_type = AP_ARG_TYPE_OPT;
  par->current->opt_short = short_opt;
  par->current->name = long_opt;
  return 0;
}

int ap_begin_sub(ap *par, const char *metavar) {
  int err = 0;
  if ((err = ap_begin(par)))
    return err;
  par->current->arg_type = AP_ARG_TYPE_SUB;
  par->current->name = metavar;
  par->current->user = NULL;
  return 0;
}

int ap_sub_add(ap *par, const char *name, ap **subpar) {
  int err = 0;
  ap_sub *sub = par->ctxcb->malloc(par->ctxcb->uptr, sizeof(ap_sub));
  if (!sub)
    return AP_ERR_NOMEM;
  if ((err = ap_init_full(subpar, NULL, par->ctxcb))) {
    par->ctxcb->free(par->ctxcb->uptr, sub);
    return err;
  }
  sub->identifier = name;
  sub->next = par->current->user;
  sub->par = *subpar;
  par->current->user = sub;
  return 0;
}

void ap_end(ap *par) {
  /* if this fails, there isn't a matching ap_begin_xxx call */
  assert(par->current);
  /* if this fails, there wasn't an ap_type_xxx call on the argument */
  assert((par->current->arg_type == AP_ARG_TYPE_OPT ||
          par->current->arg_type == AP_ARG_TYPE_POS)
             ? !!par->current->cb
             : 1);
  par->current = NULL;
}

void ap_type_custom(ap *par, ap_cb callback, void *user) {
  par->current->cb = callback;
  par->current->user = user;
}

int ap_flag_cb(void *uptr, ap_cb_data *pdata) {
  int *out = (int *)uptr;
  (void)(pdata);
  *out = 1;
  return 0;
}

void ap_type_flag(ap *par, int *out) {
  ap_type_custom(par, ap_flag_cb, (void *)out);
}

int ap_int_cb(void *uptr, ap_cb_data *pdata) {
  sscanf(pdata->arg, "%i", (int *)uptr);
  return pdata->arg_len;
}

void ap_type_int(ap *par, int *out) {
  ap_type_custom(par, ap_int_cb, (void *)out);
}

typedef struct parser {
  int argc;
  const char *const *argv;
  int idx;
  int arg_idx;
  int arg_len;
} parser;

void parser_init(parser *ctx, int argc, const char *const *argv) {
  ctx->argc = argc;
  ctx->argv = argv;
  ctx->idx = 0;
  ctx->arg_idx = 0;
  ctx->arg_len = (ctx->idx == ctx->argc) ? 0 : (int)strlen(ctx->argv[ctx->idx]);
}

void parser_advance(parser *ctx, int amt) {
  if (!amt)
    return;
  /* if this fails, you tried to run the parser backwards. this isn't possible
   * at the moment. */
  assert(amt > 0);
  /* if this fails, you tried to get more chars after exhausting input args. */
  assert(ctx->idx < ctx->argc);
  /* if this fails, you asked for too many characters from the same argument. */
  assert(amt <= (ctx->arg_len - ctx->arg_idx));
  ctx->arg_idx += amt;
  if (ctx->arg_idx == ctx->arg_len) {
    ctx->idx++;
    ctx->arg_idx = 0;
    ctx->arg_len =
        (ctx->idx == ctx->argc) ? 0 : (int)strlen(ctx->argv[ctx->idx]);
  }
}

const char *cur_arg(parser *ctx) {
  return (ctx->idx == ctx->argc || ctx->arg_idx == ctx->arg_len)
             ? NULL
             : ctx->argv[ctx->idx] + ctx->arg_idx;
}

int ap_parse_internal_part(ap_arg *arg, parser *ctx) {
  int cb_ret, cb_sub_idx = 0;
  ap_cb_data cbd = {0};
  do {
    cbd.arg = cur_arg(ctx);
    cbd.arg_len = cbd.arg ? ctx->arg_len : 0;
    cbd.idx = cb_sub_idx++;
    cbd.more = 0;
    cb_ret = arg->cb(arg->user, &cbd);
    if (cb_ret < 0)
      /* callback encountered error in parse */
      return cb_ret;
    /* callbacks should always only parse up to end of string */
    assert(cb_ret <= cbd.arg_len);
    cbd.idx++;
    parser_advance(ctx, cb_ret);
  } while (cbd.more);
  return AP_ERR_NONE;
}

ap_arg *find_next_positional_like(ap_arg *arg) {
  while (arg && arg->arg_type != AP_ARG_TYPE_POS &&
         arg->arg_type != AP_ARG_TYPE_SUB) {
    arg = arg->next;
  }
  return arg;
}

int ap_parse_internal(ap *par, parser *ctx) {
  int err;
  ap_arg *next_positional_like = find_next_positional_like(par->args);
  while (ctx->idx < ctx->argc) {
    if (cur_arg(ctx)[0] == '-' && (cur_arg(ctx)[1] && cur_arg(ctx)[1] != '-')) {
      /* optional "-O..." */
      int saved_idx = ctx->idx;
      parser_advance(ctx, 1);
      while (ctx->idx == saved_idx && cur_arg(ctx) && *cur_arg(ctx)) {
        /* accumulate chained short opts */
        ap_arg *search = par->args;
        char opt_short = *cur_arg(ctx);
        while (search) {
          if (search->arg_type == AP_ARG_TYPE_OPT &&
              search->opt_short == opt_short) {
            /* found arg with matching short opt */
            /* step over option char */
            parser_advance(ctx, 1);
            if ((err = ap_parse_internal_part(search, ctx)) < 0)
              return err;
            /* if this fails, your callback advanced to the next argument, but
             * did not fully consume that argument */
            assert(ctx->idx != saved_idx ? !ctx->arg_idx : 1);
            break;
          }
          search = search->next;
        }
        if (!search)
          /* arg not found */
          return AP_ERR_PARSE;
        /* arg found and parsing must continue */
        continue;
      }
    } else if (cur_arg(ctx)[0] == '-' && cur_arg(ctx)[1] == '-' &&
               cur_arg(ctx)[2]) {
      /* long optional "--option..."*/
      ap_arg *search = par->args;
      parser_advance(ctx, 2);
      while (search) {
        if (search->arg_type == AP_ARG_TYPE_OPT &&
            !strcmp(search->name, cur_arg(ctx))) {
          /* found arg with matching long opt */
          int prev_idx = ctx->idx;
          /* step over long opt name */
          parser_advance(ctx, (int)strlen(search->name));
          if ((err = ap_parse_internal_part(search, ctx)) < 0)
            return err;
          /* if this fails, your callback did not consume every character of the
           * argument (it returned a value less than the argument length) */
          assert(ctx->idx != prev_idx);
          break;
        }
        search = search->next;
      }
      if (!search)
        /* arg not found */
        return AP_ERR_PARSE;
      /* arg found and parsing must continue */
      continue;
    } else if (next_positional_like->arg_type == AP_ARG_TYPE_POS) {
      /* positional, includes "-" and "--" and "" */
      int part_ret = 0, prev_idx = ctx->idx;
      if ((part_ret = ap_parse_internal_part(next_positional_like, ctx)) < 0)
        return part_ret;
      /* if this fails, your callback did not consume every character of the
       * argument (it returned a value less than the argument length) */
      assert(ctx->idx != prev_idx);
      next_positional_like =
          find_next_positional_like(next_positional_like->next);
    } else if (next_positional_like->arg_type == AP_ARG_TYPE_SUB) {
      /* subparser */
      ap_sub *it = next_positional_like->user;
      while (it) {
        if (!strcmp(it->identifier, cur_arg(ctx)))
          goto found;
        it = it->next;
      }
      /* couldn't find named subparser */
      return AP_ERR_PARSE;
    found:
      if ((err = ap_parse(it->par, ctx->argc - ctx->arg_idx,
                          ctx->argv + ctx->arg_idx)))
        return err;
      next_positional_like =
          find_next_positional_like(next_positional_like->next);
    } else if (!next_positional_like) {
      /* no more positional args */
      return AP_ERR_PARSE;
    }
  }
  if (next_positional_like)
    return AP_ERR_PARSE;
  return AP_ERR_NONE;
}

int ap_parse(ap *par, int argc, const char *const *argv) {
  parser parser;
  parser_init(&parser, argc, argv);
  return ap_parse_internal(par, &parser);
}
