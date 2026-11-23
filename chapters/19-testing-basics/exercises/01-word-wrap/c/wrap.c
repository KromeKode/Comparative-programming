#include "wrap.h"

#include <stdlib.h>
#include <string.h>

char *wrap(const char *text, size_t width) {
    char *out = malloc(2 * strlen(text) + 2);
    if (out == NULL) return NULL;
    size_t length = 0;       // of out so far
    size_t line_length = 0;  // of the line at the end of out
    const char *p = text;
    while (*p != '\0') {
        while (*p == ' ') p++;
        if (*p == '\0') break;
        size_t word = strcspn(p, " ");
        if (line_length == 0) {
            // The word starts the line, even if it is longer than width.
        } else if (line_length + 1 + word <= width) {
            out[length++] = ' ';
            line_length++;
        } else {
            out[length++] = '\n';
            line_length = 0;
        }
        memcpy(out + length, p, word);
        length += word;
        line_length += word;
        p += word;
    }
    if (line_length > 0) out[length++] = '\n';
    out[length] = '\0';
    return out;
}
