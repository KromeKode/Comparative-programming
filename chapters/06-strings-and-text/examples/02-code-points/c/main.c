#include <stdio.h>
#include <string.h>

// Decodes the code point that starts at p, stores it in *code_point, and returns how
// many bytes it used. It assumes valid UTF-8; chapter 36 shows how to check.
static int decode_utf8(const unsigned char *p, unsigned long *code_point) {
    if (p[0] < 0x80) {  // 0xxxxxxx: one byte, plain ASCII
        *code_point = p[0];
        return 1;
    }
    if (p[0] < 0xE0) {  // 110xxxxx 10xxxxxx
        *code_point = ((p[0] & 0x1Ful) << 6) | (p[1] & 0x3Ful);
        return 2;
    }
    if (p[0] < 0xF0) {  // 1110xxxx 10xxxxxx 10xxxxxx
        *code_point = ((p[0] & 0x0Ful) << 12) | ((p[1] & 0x3Ful) << 6) | (p[2] & 0x3Ful);
        return 3;
    }
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    *code_point = ((p[0] & 0x07ul) << 18) | ((p[1] & 0x3Ful) << 12) |
                  ((p[2] & 0x3Ful) << 6) | (p[3] & 0x3Ful);
    return 4;
}

int main(void) {
    const char *s = "Zo\u00eb \U0001F44D\U0001F3FD";
    const unsigned char *bytes = (const unsigned char *)s;
    size_t length = strlen(s);

    printf("UTF-8 bytes:");
    for (size_t i = 0; i < length; i++) {
        printf(" %02X", bytes[i]);
    }
    printf("\n");

    printf("code points:");
    for (size_t i = 0; i < length;) {
        unsigned long code_point;
        i += (size_t)decode_utf8(bytes + i, &code_point);
        printf(" U+%04lX", code_point);
    }
    printf("\n");

    // Find where the third code point ends, then print that many bytes.
    size_t end = 0;
    for (int n = 0; n < 3; n++) {
        unsigned long ignored;
        end += (size_t)decode_utf8(bytes + end, &ignored);
    }
    printf("first 3 code points: %.*s\n", (int)end, s);
    return 0;
}
