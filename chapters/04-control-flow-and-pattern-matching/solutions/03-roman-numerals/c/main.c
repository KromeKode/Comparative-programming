#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int VALUES[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
static const char *const SYMBOLS[] = {"M",  "CM", "D",  "CD", "C",  "XC", "L",
                                      "XL", "X",  "IX", "V",  "IV", "I"};

static void print_roman(int n) {
    for (size_t i = 0; i < sizeof VALUES / sizeof VALUES[0]; i++) {
        while (n >= VALUES[i]) {  // take the biggest value that still fits
            fputs(SYMBOLS[i], stdout);
            n -= VALUES[i];
        }
    }
}

static int letter_value(char letter) {
    switch (letter) {
    case 'I':
        return 1;
    case 'V':
        return 5;
    case 'X':
        return 10;
    case 'L':
        return 50;
    case 'C':
        return 100;
    case 'D':
        return 500;
    case 'M':
        return 1000;
    default:
        return 0;  // also covers the '\0' after the last letter
    }
}

static int from_roman(const char *text) {
    int total = 0;
    for (size_t i = 0; text[i] != '\0'; i++) {
        int value = letter_value(text[i]);
        // A smaller letter before a bigger one is subtracted: IV is 5 - 1.
        if (value < letter_value(text[i + 1])) {
            total -= value;
        } else {
            total += value;
        }
    }
    return total;
}

int main(void) {
    char line[64];
    while (fgets(line, sizeof line, stdin) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') {
            continue;
        }
        if (isdigit((unsigned char)line[0])) {
            int n = atoi(line);
            printf("%d -> ", n);
            print_roman(n);
            printf("\n");
        } else {
            printf("%s -> %d\n", line, from_roman(line));
        }
    }
    return 0;
}
