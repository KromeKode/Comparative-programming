#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *yes_no(int condition) { return condition ? "yes" : "no"; }

// strcmp compares bytes, so every uppercase ASCII letter sorts before every lowercase one.
static int compare_bytes(const void *a, const void *b) {
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static int compare_ignoring_case(const void *a, const void *b) {
    const unsigned char *x = *(const unsigned char *const *)a;
    const unsigned char *y = *(const unsigned char *const *)b;
    while (*x != '\0' && tolower(*x) == tolower(*y)) {
        x++;
        y++;
    }
    return tolower(*x) - tolower(*y);
}

static void print_list(const char *label, const char **items, size_t count) {
    printf("%s:", label);
    for (size_t i = 0; i < count; i++) {
        printf(" %s", items[i]);
    }
    printf("\n");
}

int main(void) {
    const char *text = "  The quick  brown fox  ";

    // Trim: skip spaces at the front, then find the last non-space.
    const char *start = text;
    while (isspace((unsigned char)*start)) {
        start++;
    }
    size_t length = strlen(start);
    while (length > 0 && isspace((unsigned char)start[length - 1])) {
        length--;
    }
    char trimmed[64];
    snprintf(trimmed, sizeof trimmed, "%.*s", (int)length, start);
    printf("trimmed: [%s]\n", trimmed);

    // Split on runs of spaces by walking the string.
    printf("words: ");
    int first = 1;
    for (const char *p = trimmed; *p != '\0';) {
        while (isspace((unsigned char)*p)) {
            p++;
        }
        const char *word = p;
        while (*p != '\0' && !isspace((unsigned char)*p)) {
            p++;
        }
        printf("%s%.*s", first ? "" : "|", (int)(p - word), word);
        first = 0;
    }
    printf("\n");

    char upper[64];
    for (size_t i = 0; i <= length; i++) {  // <= copies the '\0' too
        upper[i] = (char)toupper((unsigned char)trimmed[i]);
    }
    printf("upper: %s\n", upper);

    const char *found = strstr(trimmed, "brown");  // NULL if not found
    printf("contains \"quick\": %s\n", yes_no(strstr(trimmed, "quick") != NULL));
    printf("index of \"brown\": %td\n", found - trimmed);

    // Replace: copy the part before the match, the new text, then the rest.
    const char *match = strstr(trimmed, "quick");
    char replaced[64];
    snprintf(replaced, sizeof replaced, "%.*s%s%s", (int)(match - trimmed), trimmed, "slow",
             match + strlen("quick"));
    printf("replaced: %s\n", replaced);

    printf("starts with \"The\": %s\n", yes_no(strncmp(trimmed, "The", 3) == 0));

    const char *words[] = {"banana", "apple", "Cherry"};
    qsort(words, 3, sizeof words[0], compare_bytes);
    print_list("sorted", words, 3);
    qsort(words, 3, sizeof words[0], compare_ignoring_case);
    print_list("sorted ignoring case", words, 3);
    return 0;
}
