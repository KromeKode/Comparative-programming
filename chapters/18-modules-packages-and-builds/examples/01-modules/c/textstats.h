// The interface of the textstats module: what other files may use. A file that
// uses the module includes this header, so it holds declarations, not code.
//
// #include pastes the header's text in, and a file can end up including it
// twice, through two other headers. The include guard makes the second copy
// empty: TEXTSTATS_H is defined the first time, and #ifndef skips the rest.
#ifndef TEXTSTATS_H
#define TEXTSTATS_H

#include <stddef.h>

#define TEXTSTATS_VERSION "1.0"

// The number of words in text. A word is a run of ASCII letters.
size_t word_count(const char *text);

// Copies the first of the longest words in text into out, which has room for
// size bytes, cutting it short if it doesn't fit.
void longest_word(const char *text, char *out, size_t size);

#endif
