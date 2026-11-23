#ifndef DURATION_H
#define DURATION_H

#include <stdbool.h>
#include <stddef.h>

// Reads a duration such as "1h30m" into *seconds, as 5400. Returns false, and
// leaves *seconds alone, if text isn't a duration or is too long to count.
bool parse_duration(const char *text, long long *seconds);

// Writes seconds, which is at least 0, as a duration such as "1h30m" into out,
// which has room for size characters. Returns false if it doesn't fit.
bool format_duration(long long seconds, char *out, size_t size);

#endif
