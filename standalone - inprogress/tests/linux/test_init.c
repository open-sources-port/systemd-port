#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <linux/init.h>

// Simple initcall function for testing
static int test_initcall_success(void) {
    printf("test_initcall_success called\n");
    return 0; // success
}

static int test_initcall_failure(void) {
    printf("test_initcall_failure called\n");
    return -1; // failure
}

int main(void) {
    int ret;

    // Test success case
    ret = do_one_initcall(test_initcall_success);
    assert(ret == 0);
    printf("Success initcall passed\n");

    // Test failure case
    ret = do_one_initcall(test_initcall_failure);
    assert(ret != 0);
    printf("Failure initcall passed\n");

    printf("All tests passed!\n");
    return 0;
}
