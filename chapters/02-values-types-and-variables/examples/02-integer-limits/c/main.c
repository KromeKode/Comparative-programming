#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    int32_t i32 = INT32_MAX;
    printf("int32 max: %" PRId32 "\n", i32);

    // i32 + 1 would be undefined behavior: signed overflow is not allowed in C.
    // GCC and Clang provide a checked add that reports overflow instead.
    int32_t sum;
    if (__builtin_add_overflow(i32, 1, &sum)) {
        printf("int32 max + 1: overflow\n");
    }

    uint32_t u32 = UINT32_MAX;  // unsigned types are defined to wrap around
    u32 += 1;
    printf("uint32 max + 1: %" PRIu32 "\n", u32);

    printf("int64 max: %" PRId64 "\n", INT64_MAX);
    return 0;
}
