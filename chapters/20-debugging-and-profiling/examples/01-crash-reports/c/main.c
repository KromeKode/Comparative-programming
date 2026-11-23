// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
#include <ctype.h>
#include <stdio.h>

static void count_letters(FILE *in, int counts[26]) {
    int c;
    while ((c = getc(in)) != EOF) {
        if (isalpha(c)) counts[c - 'a']++;
    }
}

int main(void) {
    int counts[26] = {0};
    count_letters(stdin, counts);
    for (int i = 0; i < 26; i++) {
        if (counts[i] == 0) continue;
        printf("%c %d ", 'a' + i, counts[i]);
        for (int j = 0; j < counts[i]; j++) putchar('#');
        putchar('\n');
    }
    return 0;
}
