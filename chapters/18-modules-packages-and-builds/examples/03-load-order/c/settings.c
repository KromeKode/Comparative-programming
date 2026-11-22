#include "settings.h"

#include <stdio.h>

const char *settings_greeting(void) {
    // A static variable inside a function keeps its value between calls.
    static const char *greeting = NULL;
    if (greeting == NULL) {
        puts("settings: loading");
        greeting = "Hello";
    }
    return greeting;
}
