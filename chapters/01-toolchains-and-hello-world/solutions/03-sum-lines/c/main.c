#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char line[256];
    long long total = 0;  // 64 bits; a plain int would overflow on the big test
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *end;
        long long value = strtoll(line, &end, 10);  // skips leading whitespace
        if (end == line) {
            continue;  // nothing to parse: a blank line
        }
        total += value;
    }
    printf("%lld\n", total);
    return 0;
}
