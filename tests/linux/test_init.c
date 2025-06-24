#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <linux/init.h>

/* Dummy initcall function */
static int test_initcall_success(void) {
    return 0;
}

static int test_initcall_fail(void) {
    return -1;
}

/* Test do_one_initcall with success */
static void test_do_one_initcall_success(void **state) {
    (void) state; // unused

    int ret = do_one_initcall(test_initcall_success);
    assert_int_equal(ret, 0);
}

/* Test do_one_initcall with failure */
static void test_do_one_initcall_fail(void **state) {
    (void) state;

    int ret = do_one_initcall(test_initcall_fail);
    assert_int_not_equal(ret, 0);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_do_one_initcall_success),
        cmocka_unit_test(test_do_one_initcall_fail),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
