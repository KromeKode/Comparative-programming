#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Shifts an ASCII letter and leaves every other byte alone. That is safe for UTF-8:
// every byte of a non-ASCII character is 0x80 or above, so none looks like a letter.
static char shift_letter(char c, int shift) {
    if (c >= 'a' && c <= 'z') {
        return (char)('a' + (c - 'a' + shift) % 26);
    }
    if (c >= 'A' && c <= 'Z') {
        return (char)('A' + (c - 'A' + shift) % 26);
    }
    return c;
}

int main(void) {
    char line[1024];
    while (fgets(line, sizeof line, stdin) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';  // drop the line ending
        char *end;
        long shift = strtol(line, &end, 10);  // end points just past the number
        if (end == line) {
            continue;  // a blank line: no number to read
        }
        // C's % keeps the sign of the left side, so turn the shift into 0 to 25 first.
        int positive = (int)(((shift % 26) + 26) % 26);
        char *text = *end == ' ' ? end + 1 : end;
        for (char *p = text; *p != '\0'; p++) {
            *p = shift_letter(*p, positive);
        }
        printf("%s\n", text);
    }
    return 0;
}
