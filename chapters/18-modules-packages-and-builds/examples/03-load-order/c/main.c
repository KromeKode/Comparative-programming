#include <stdio.h>

#include "settings.h"

int main(void) {
    puts("main: start");
    printf("main: the greeting is %s\n", settings_greeting());
    printf("main: the greeting is still %s\n", settings_greeting());
    return 0;
}
