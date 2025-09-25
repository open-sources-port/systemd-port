#include <stdio.h>
#include <assert.h>
#include <linux/compiler.h>

int main(void) {
    // Check struct size: should be 3 bytes (1 + 2)
    assert(sizeof(struct PackedData) == 3);

    // Test assigning values and reading back
    struct PackedData pd;
    pd.a = 0x12;
    pd.b = 0x3456;

    assert(pd.a == 0x12);
    assert(pd.b == 0x3456);

    // Print results for manual verification
    printf("PackedData.a = 0x%x\n", pd.a);
    printf("PackedData.b = 0x%x\n", pd.b);
    printf("Size of PackedData = %zu bytes\n", sizeof(pd));

    printf("All tests passed.\n");
    return 0;
}
