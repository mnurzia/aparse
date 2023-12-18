#include <aparse.h>
#include <string.h>

#define MPTEST_IMPLEMENTATION
#include "mptest.h"

TEST(init) {
  ap *parser = ap_init("test");
  ap_destroy(parser);
  PASS();
}

TEST(no_args_unspecified) {
  ap *parser = ap_init("test");
  int err = 0;
  const char *const argv[] = {NULL};
  if (!parser)
    goto done;
  if ((err = ap_parse(parser, 0, argv) == AP_ERR_NOMEM))
    goto done;
  ASSERT(!err);
done:
  ap_destroy(parser);
  PASS();
}

TEST(opt_short_only) {
  ap *parser = ap_init("test");
  int err = 0;
  int flag = -1;
  int argc = 1;
  const char *const argv[] = {"-O", NULL};
  if (!parser)
    goto done;
  if (ap_opt(parser, 'O', NULL))
    goto done;
  ap_type_flag(parser, &flag);
  if ((err = ap_parse(parser, argc, argv) == AP_ERR_NOMEM))
    goto done;
  ASSERT(!err);
  ASSERT_EQ(flag, 1);
done:
  ap_destroy(parser);
  PASS();
}

TEST(opt_long_only) {
  ap *parser = ap_init("test");
  int err = 0;
  int flag = -1;
  int argc = 1;
  const char *const argv[] = {"--option", NULL};
  if (!parser)
    goto done;
  if (ap_opt(parser, 0, "option"))
    goto done;
  ap_type_flag(parser, &flag);
  if ((err = ap_parse(parser, argc, argv) == AP_ERR_NOMEM))
    goto done;
  ASSERT(!err);
  ASSERT_EQ(flag, 1);
done:
  ap_destroy(parser);
  PASS();
}

TEST(opt_unspecified) {
  ap *parser = ap_init("test");
  int err = 0;
  int flag = -1;
  int argc = 0;
  const char *const argv[] = {NULL};
  if (!parser)
    goto done;
  if (ap_opt(parser, 'O', "option"))
    goto done;
  ap_type_flag(parser, &flag);
  if ((err = ap_parse(parser, argc, argv) == AP_ERR_NOMEM))
    goto done;
  ASSERT(!err);
  ASSERT_EQ(flag, -1);
done:
  ap_destroy(parser);
  PASS();
}

TEST(opt_short_specified) {
  ap *parser = ap_init("test");
  int err = 0;
  int flag = -1;
  int argc = 1;
  const char *const argv[] = {"-O", NULL};
  if (!parser)
    goto done;
  if (ap_opt(parser, 'O', "option"))
    goto done;
  ap_type_flag(parser, &flag);
  if ((err = ap_parse(parser, argc, argv) == AP_ERR_NOMEM))
    goto done;
  ASSERT(!err);
  ASSERT_EQ(flag, 1);
done:
  ap_destroy(parser);
  PASS();
}

TEST(opt_long_specified) {
  ap *parser = ap_init("test");
  int err = 0;
  int flag = -1;
  int argc = 1;
  const char *const argv[] = {"--option", NULL};
  if (!parser)
    goto done;
  if (ap_opt(parser, 'O', "option"))
    goto done;
  ap_type_flag(parser, &flag);
  if ((err = ap_parse(parser, argc, argv) == AP_ERR_NOMEM))
    goto done;
  ASSERT(!err);
  ASSERT_EQ(flag, 1);
done:
  ap_destroy(parser);
  PASS();
}

TEST(pos_specified) {
  ap *parser = ap_init("test");
  int err = 0;
  int flag = -1;
  int argc = 1;
  const char *const argv[] = {"1", NULL};
  if (!parser)
    goto done;
  if (ap_pos(parser, "option"))
    goto done;
  ap_type_int(parser, &flag);
  if ((err = ap_parse(parser, argc, argv) == AP_ERR_NOMEM))
    goto done;
  ASSERT(!err);
  ASSERT_EQ(flag, 1);
done:
  ap_destroy(parser);
  PASS();
}

TEST(sub_empty) {
  ap *parser = ap_init("test");
  int err = 0;
  int argc = 1;
  int out_idx = 0;
  int arg = 0;
  const char *const argv[] = {"1"};
  if (!parser)
    goto done;
  if (ap_pos(parser, "command"))
    goto done;
  ap_type_sub(parser, "command", &out_idx);
  {
    ap *sub;
    if (ap_sub_add(parser, NULL, &sub))
      goto done;
    if (ap_pos(sub, "test"))
      goto done;
    ap_type_int(sub, &arg);
  }
  if ((err = ap_parse(parser, argc, argv)) == AP_ERR_NOMEM)
    goto done;
  ASSERT(!err);
  ASSERT(arg == 1);
done:
  ap_destroy(parser);
  PASS();
}

struct bufs {
  char out[2048];
  char err[2048];
};

int dummy_print_cb(void *uptr, int fd, const char *text, size_t n) {
  if (uptr) {
    if (fd == AP_FD_OUT)
      strncat(((struct bufs *)uptr)->out, text, n);
    else
      strncat(((struct bufs *)uptr)->err, text, n);
  }
  return AP_ERR_NONE;
}

ap *make_out_hooks(ap_ctxcb *cb, struct bufs *bufs) {
  ap *parser;
  int err = AP_ERR_NONE;
  cb->uptr = bufs;
  cb->print = dummy_print_cb;
  if ((err = ap_init_full(&parser, "abc", cb)))
    return NULL;
  return parser;
}

TEST(usage_empty) {
  ap_ctxcb cb = {0};
  struct bufs b = {0};
  ap *parser = make_out_hooks(&cb, &b);
  if (!parser)
    goto done;
  ASSERT(!ap_show_usage(parser));
  ASSERT(!strcmp(b.out, "usage: abc"));
done:
  ap_destroy(parser);
  PASS();
}

TEST(help_empty) {
  ap_ctxcb cb = {0};
  struct bufs b = {0};
  ap *parser = make_out_hooks(&cb, &b);
  if (!parser)
    goto done;
  ASSERT(!ap_show_help(parser));
  ASSERT(!strcmp(b.out, "usage: abc\n"));
done:
  ap_destroy(parser);
  PASS();
}

TEST(help_opts) {
  ap_ctxcb cb = {0};
  struct bufs b = {0};
  ap *parser = make_out_hooks(&cb, &b);
  int dummy_flag;
  if (!parser)
    goto done;
  ap_description(parser, "description");
  ap_epilog(parser, "epilog");
  if (ap_opt(parser, 'o', "opt"))
    goto done;
  ap_type_flag(parser, &dummy_flag);
  ap_help(parser, "option");
  ASSERT(!ap_show_help(parser));
  ASSERT(!strcmp(b.out,
                 "usage: abc [-o]\n\ndescription\n\noptional arguments:\n  -o,"
                 "--opt\n    option\n\nepilog\n"));
done:
  ap_destroy(parser);
  PASS();
}

TEST(type_enum) {
  ap_ctxcb cb = {0};
  struct bufs b = {0};
  ap *parser = make_out_hooks(&cb, &b);
  int flag;
  const char *enum_choices[] = {"a", "bcd", NULL};
  int argc = 2;
  const char *const argv[] = {"-e", "bcd"};
  if (!parser)
    goto done;
  if (ap_opt(parser, 'e', "enum"))
    goto done;
  if (ap_type_enum(parser, &flag, enum_choices))
    goto done;
  ap_help(parser, "option");
  ASSERT(!ap_show_help(parser));
  ASSERT(!strcmp(b.out,
                 "usage: abc [-e {a,bcd}]\n\noptional arguments:\n  -e {a,bcd},"
                 "--enum {a,bcd}\n    option\n"));
  ASSERT(!ap_parse(parser, argc, argv));
  ASSERT(flag == 1);
done:
  ap_destroy(parser);
  PASS();
}

int main(int argc, const char *const *argv) {
  MPTEST_MAIN_BEGIN_ARGS(argc, argv);
  RUN_TEST(init);
  RUN_TEST(no_args_unspecified);
  RUN_TEST(opt_unspecified);
  RUN_TEST(opt_short_specified);
  RUN_TEST(opt_short_only);
  RUN_TEST(opt_long_only);
  RUN_TEST(opt_long_specified);
  RUN_TEST(pos_specified);
  RUN_TEST(sub_empty);
  RUN_TEST(usage_empty);
  RUN_TEST(help_empty);
  RUN_TEST(help_opts);
  RUN_TEST(type_enum);
  MPTEST_MAIN_END();
}
