#include <stdio.h>
#include <string.h>

int main(void) {
    char line[256];
    if (fgets(line, sizeof line, stdin) == NULL) {
        line[0] = '\0';                  // no input at all
    }
    line[strcspn(line, "\r\n")] = '\0';  // cut the line ending off
    printf("You typed: %s\n", line);
    return 0;
}
