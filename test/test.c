#include <aparse.h>

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
  if (ap_begin_opt(parser, 'O', NULL))
    goto done;
  ap_type_flag(parser, &flag);
  ap_end(parser);
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
  if (ap_begin_opt(parser, 0, "option"))
    goto done;
  ap_type_flag(parser, &flag);
  ap_end(parser);
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
  if (ap_begin_opt(parser, 'O', "option"))
    goto done;
  ap_type_flag(parser, &flag);
  ap_end(parser);
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
  if (ap_begin_opt(parser, 'O', "option"))
    goto done;
  ap_type_flag(parser, &flag);
  ap_end(parser);
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
  if (ap_begin_opt(parser, 'O', "option"))
    goto done;
  ap_type_flag(parser, &flag);
  ap_end(parser);
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
  if (ap_begin_pos(parser, "option"))
    goto done;
  ap_type_int(parser, &flag);
  ap_end(parser);
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
  int argc = 0;
  const char *const argv[] = {NULL};
  if (!parser)
    goto done;
  if (ap_begin_sub(parser, "command"))
    goto done;
  ap_end(parser);
  if ((err = ap_parse(parser, argc, argv)) == AP_ERR_NOMEM)
    goto done;
  ASSERT(err == AP_ERR_PARSE);
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
  MPTEST_MAIN_END();
}
