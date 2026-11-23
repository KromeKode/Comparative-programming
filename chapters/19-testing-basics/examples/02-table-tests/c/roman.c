#include "roman.h"

#include <string.h>

static const struct {
    int value;
    const char *letters;
} NUMERALS[] = {
    {1000, "M"},
    {900, "CM"},
    {500, "D"},
    {400, "CD"},
    {100, "C"},
    {90, "XC"},
    {50, "L"},
    {40, "XL"},
    {10, "X"},
    {9, "IX"},
    {5, "V"},
    {4, "IV"},
    {1, "I"},
};

bool to_roman(int n, char *out, size_t size) {
    if (n < 1 || n > 3999) return false;
    char numeral[16];
    size_t length = 0;
    for (size_t i = 0; i < sizeof NUMERALS / sizeof NUMERALS[0]; i++) {
        for (; n >= NUMERALS[i].value; n -= NUMERALS[i].value) {
            size_t letters = strlen(NUMERALS[i].letters);
            memcpy(numeral + length, NUMERALS[i].letters, letters);
            length += letters;
        }
    }
    if (length + 1 > size) return false;
    numeral[length] = '\0';
    memcpy(out, numeral, length + 1);
    return true;
}
