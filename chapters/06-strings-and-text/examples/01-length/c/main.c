#include <stdio.h>
#include <string.h>

// In UTF-8, every byte of the form 10xxxxxx continues a code point, and every other
// byte starts one. So counting the bytes that are not 10xxxxxx counts code points.
static size_t count_code_points(const char *s) {
    size_t count = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; p++) {
        if ((*p & 0xC0) != 0x80) {
            count++;
        }
    }
    return count;
}

// A code point above U+FFFF needs 4 bytes in UTF-8 (first byte 11110xxx) and two
// code units in UTF-16. Everything else needs one UTF-16 code unit.
static size_t count_utf16_units(const char *s) {
    size_t count = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p != '\0'; p++) {
        if ((*p & 0xC0) != 0x80) {
            count += *p >= 0xF0 ? 2 : 1;
        }
    }
    return count;
}

int main(void) {
    // \u and \U name a code point; GCC and Clang store the string as UTF-8.
    const char *s = "Zo\u00eb \U0001F44D\U0001F3FD";

    printf("text: %s\n", s);
    printf("built-in length: %zu (bytes)\n", strlen(s));
    printf("UTF-8 bytes: %zu\n", strlen(s));
    printf("UTF-16 code units: %zu\n", count_utf16_units(s));
    printf("code points: %zu\n", count_code_points(s));
    return 0;
}
