#ifndef WRAP_H
#define WRAP_H

#include <stddef.h>

// Wraps text into lines of at most width characters, breaking only at spaces.
// Returns the lines, each ending in '\n', in one string from malloc, which the
// caller frees, or NULL if there is no memory. Text with no words gives "".
char *wrap(const char *text, size_t width);

#endif
