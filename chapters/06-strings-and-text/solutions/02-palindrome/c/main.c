#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Decodes the code point that starts at p and returns how many bytes it used.
// It assumes valid UTF-8, like example 02-code-points.
static int decode_utf8(const unsigned char *p, unsigned long *code_point) {
    if (p[0] < 0x80) {
        *code_point = p[0];
        return 1;
    }
    if (p[0] < 0xE0) {
        *code_point = ((p[0] & 0x1Ful) << 6) | (p[1] & 0x3Ful);
        return 2;
    }
    if (p[0] < 0xF0) {
        *code_point = ((p[0] & 0x0Ful) << 12) | ((p[1] & 0x3Ful) << 6) | (p[2] & 0x3Ful);
        return 3;
    }
    *code_point = ((p[0] & 0x07ul) << 18) | ((p[1] & 0x3Ful) << 12) |
                  ((p[2] & 0x3Ful) << 6) | (p[3] & 0x3Ful);
    return 4;
}

// Reversing the bytes would scramble every multi-byte character, so decode the line
// into code points first and compare those.
static int is_palindrome(const char *line) {
    unsigned long kept[1024];
    size_t count = 0;
    const unsigned char *p = (const unsigned char *)line;
    while (*p != '\0' && count < sizeof kept / sizeof kept[0]) {
        unsigned long cp;
        p += decode_utf8(p, &cp);
        if (cp >= 0x80) {
            kept[count++] = cp;  // keep non-ASCII characters as they are
        } else if (isalnum((int)cp)) {
            kept[count++] = (unsigned long)tolower((int)cp);
        }
    }
    for (size_t i = 0; i < count / 2; i++) {
        if (kept[i] != kept[count - 1 - i]) {
            return 0;
        }
    }
    return 1;
}

int main(void) {
    char line[4096];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *start = line;
        while (isspace((unsigned char)*start)) {
            start++;
        }
        size_t length = strlen(start);
        while (length > 0 && isspace((unsigned char)start[length - 1])) {
            start[--length] = '\0';
        }
        if (length == 0) {
            continue;
        }
        printf("%s: %s\n", start, is_palindrome(start) ? "palindrome" : "not a palindrome");
    }
    return 0;
}
