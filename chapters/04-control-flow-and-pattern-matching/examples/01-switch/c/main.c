#include <stdio.h>

static const char *describe(int code) {
    const char *text = "unknown";
    switch (code) {
    case 200:
        text = "ok";
        break;  // without break, execution falls into the next case
    case 301:
    case 302:
    case 307:
    case 308:  // several labels can share one body
        text = "redirect";
        break;
    case 404:
        text = "not found";
        break;
    default:
        // Case labels must be single constants, so ranges need an if.
        if (code >= 400 && code <= 499) {
            text = "client error";
        } else if (code >= 500 && code <= 599) {
            text = "server error";
        }
        break;
    }
    return text;
}

int main(void) {
    const int codes[] = {200, 301, 308, 404, 418, 500, 503, 99};
    for (size_t i = 0; i < sizeof codes / sizeof codes[0]; i++) {
        printf("%d: %s\n", codes[i], describe(codes[i]));
    }
    return 0;
}
