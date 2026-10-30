#include <stdio.h>

static const int grid[3][3] = {{3, 8, 1}, {9, 4, 7}, {2, 6, 5}};

int main(void) {
    const int targets[] = {7, 10};
    for (int t = 0; t < 2; t++) {
        int target = targets[t];
        int checked = 0;
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                checked++;
                if (grid[r][c] == target) {
                    printf("%d: row %d, column %d (checked %d cells)\n", target, r, c,
                           checked);
                    goto next_target;  // C has no labeled break; goto is the usual way out
                }
            }
        }
        printf("%d: not found (checked %d cells)\n", target, checked);
    next_target:;
    }
    return 0;
}
