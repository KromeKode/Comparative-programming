#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint64_t start;
    while (scanf("%" SCNu64, &start) == 1) {
        // The values climb far past 2^32 for some starts, so use 64 bits.
        uint64_t n = start;
        uint64_t peak = n;
        int steps = 0;
        while (n != 1) {
            n = (n % 2 == 0) ? n / 2 : 3 * n + 1;
            if (n > peak) {
                peak = n;
            }
            steps++;
        }
        printf("%" PRIu64 ": steps %d, peak %" PRIu64 "\n", start, steps, peak);
    }
    return 0;
}
