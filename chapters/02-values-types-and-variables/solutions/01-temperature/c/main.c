#include <stdio.h>

int main(void) {
    char line[256];
    while (fgets(line, sizeof line, stdin) != NULL) {
        double value;
        char unit;
        if (sscanf(line, "%lf %c", &value, &unit) != 2) {
            continue;  // a blank line
        }
        if (unit == 'C') {
            printf("%.1f C = %.1f F\n", value, value * 9.0 / 5.0 + 32.0);
        } else {
            printf("%.1f F = %.1f C\n", value, (value - 32.0) * 5.0 / 9.0);
        }
    }
    return 0;
}
