#include <ctype.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    char line[1024];
    if (fgets(line, sizeof line, stdin) == NULL) {
        line[0] = '\0';
    }

    // Trim whitespace from both ends. This also removes "\n" and "\r\n".
    char *start = line;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';

    if (*start == '\0') {
        printf("Hello, stranger!\n");
    } else {
        printf("Hello, %s!\n", start);
    }
    return 0;
}
