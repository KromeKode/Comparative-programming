#ifndef ROMAN_H
#define ROMAN_H

#include <stdbool.h>
#include <stddef.h>

// Writes n in Roman numerals to out, which has room for size bytes, and returns
// true. Returns false, and writes nothing, if n is outside 1 to 3999 or the
// numeral doesn't fit. The longest, for 3888, is 15 letters.
bool to_roman(int n, char *out, size_t size);

#endif
