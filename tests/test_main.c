#define MPTEST_IMPLEMENTATION
#include "test_harness.h"

TEST(t_aparse_init) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
error:
    aparse_destroy(&ap);
    PASS();
}

TEST(t_aparse_add_opt) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
    if ((err = aparse_add_opt(&ap, 'a', "an-option"))) {
        goto error;
    }
error:
    aparse_destroy(&ap);
    PASS();
}

TEST(t_aparse_add_opt_bad) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
    ASSERT_ASSERT(aparse_add_opt(&ap, 0, NULL));
error:
    aparse_destroy(&ap);
    PASS();
}

TEST(t_aparse_add_pos) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
    if ((err = aparse_add_pos(&ap, "integer"))) {
        goto error;
    }
error:
    aparse_destroy(&ap);
    PASS();
}

TEST(t_aparse_add_sub) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
    if ((err = aparse_add_sub(&ap))) {
        goto error;
    }
error:
    aparse_destroy(&ap);
    PASS();
}

#define ARGV_SIZE(argv) sizeof(argv) / sizeof(const char*)

aparse_error dummy_out_cb(void* user, const char* buf, mn_size buf_size) {
    (void)(user);
    (void)(buf);
    (void)(buf_size);
    return 0;
}

aparse_error parse_one(aparse_state* state, const char* a) {
    const char* l[2];
    l[0] = "prog";
    l[1] = a;
    aparse_set_out_cb(state, dummy_out_cb, MN_NULL);
    return aparse_parse(state, 2, l);
}

aparse_error parse_two(aparse_state* state, const char* a, const char* b) {
    const char* l[3];
    l[0] = "prog";
    l[1] = a;
    l[2] = b;
    aparse_set_out_cb(state, dummy_out_cb, MN_NULL);
    return aparse_parse(state, 3, l);
}

TEST(t_aparse_parse_opt_bool) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    int out = 0;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
    if ((err = aparse_add_opt(&ap, 'a', "arg"))) {
        goto error;
    }
    aparse_arg_type_bool(&ap, &out);
    ASSERT(!parse_one(&ap, "-a"));
    ASSERT_EQ(out, 1);
    out = 0;
    ASSERT(!parse_one(&ap, "-a=1"));
    ASSERT_EQ(out, 1);
    out = 5;
    ASSERT(!parse_one(&ap, "-a=0"));
    ASSERT_EQ(out, 0);
    out = 0;
    ASSERT(!parse_one(&ap, "--arg"));
    ASSERT_EQ(out, 1);
    out = 0;
    ASSERT(!parse_one(&ap, "--arg=1"));
    ASSERT_EQ(out, 1);
    out = 1;
    ASSERT(!parse_one(&ap, "--arg=0"));
    ASSERT_EQ(out, 0);
    ASSERT_EQ(parse_one(&ap, "-c"), APARSE_ERROR_PARSE);
    ASSERT_EQ(parse_one(&ap, "-a=a"), APARSE_ERROR_PARSE);
    ASSERT_EQ(parse_one(&ap, "-a=$"), APARSE_ERROR_PARSE);
    ASSERT_EQ(parse_one(&ap, "-a="), APARSE_ERROR_PARSE);
    ASSERT_EQ(parse_one(&ap, "-a=abcd"), APARSE_ERROR_PARSE);
error:
    aparse_destroy(&ap);
    PASS();
}

TEST(t_aparse_parse_opt_multiple_bools) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    int a, b;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
    if ((err = aparse_add_opt(&ap, 'a', "arg"))) {
        goto error;
    }
    aparse_arg_type_bool(&ap, &a);
    if ((err = aparse_add_opt(&ap, 'b', "bar"))) {
        goto error;
    }
    aparse_arg_type_bool(&ap, &b);

    a = 0; b = 0;
    ASSERT(!parse_one(&ap, "-a"));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 0);

    a = 0; b = 0;
    ASSERT(!parse_one(&ap, "-b"));
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 1);

    a = 0; b = 0;
    ASSERT(!parse_one(&ap, "-ab"));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);

    a = 0; b = 0;
    ASSERT(!parse_one(&ap, "-ba"));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);

    a = 0; b = 0;
    ASSERT(!parse_two(&ap, "-a", "-b"));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);

    a = 0; b = 0;
    ASSERT(!parse_two(&ap, "--arg", "--bar"));
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);

    a = 1; b = 1;
    ASSERT(!parse_two(&ap, "--arg=0", "--bar=0"));
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);
error:
    aparse_destroy(&ap);
    PASS();
}

TEST(t_aparse_parse_pos_bool) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    int out = 0;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
    if ((err = aparse_add_pos(&ap, "integer"))) {
        goto error;
    }
    aparse_arg_type_bool(&ap, &out);
    
    out = 0;
    ASSERT(!parse_one(&ap, "1"));
    ASSERT_EQ(out, 1);

    out = 1;
    ASSERT(!parse_one(&ap, "0"));
    ASSERT_EQ(out, 0);

    ASSERT_EQ(parse_one(&ap, ""), APARSE_ERROR_PARSE);
error:
    aparse_destroy(&ap);
    PASS();
}

TEST(t_aparse_help) {
    aparse_error err = APARSE_ERROR_NONE;
    aparse_state ap;
    const char* help_argv[] = {"prog", "-h"};
    const char* str_out;
    if ((err = aparse_init(&ap))) {
        goto error;
    }
    if ((err = aparse_add_opt(&ap, 'h', "help"))) {
        goto error;
    }
    aparse_arg_type_help(&ap);

    if ((err = aparse_add_opt(&ap, 'v', "version"))) {
        goto error;
    }
    aparse_arg_type_version(&ap);

    if ((err = aparse_add_pos(&ap, "integer"))) {
        goto error;
    }
    aparse_arg_type_str(&ap, &str_out, NULL);

    if ((err = aparse_parse(&ap, 2, help_argv))) {
        goto error;
    }
error:
    aparse_destroy(&ap);
    PASS();
}

int main() {
    MPTEST_MAIN_BEGIN();
    MPTEST_ENABLE_LEAK_CHECKING();
    RUN_TEST(t_aparse_init);
    RUN_TEST(t_aparse_add_opt);
    RUN_TEST(t_aparse_add_opt_bad);
    RUN_TEST(t_aparse_add_pos);
    RUN_TEST(t_aparse_add_sub);
    RUN_TEST(t_aparse_parse_opt_bool);
    RUN_TEST(t_aparse_parse_opt_multiple_bools);
    RUN_TEST(t_aparse_parse_pos_bool);
    RUN_TEST(t_aparse_help);
    MPTEST_DISABLE_LEAK_CHECKING();
    MPTEST_MAIN_END();
    return 0;
}
