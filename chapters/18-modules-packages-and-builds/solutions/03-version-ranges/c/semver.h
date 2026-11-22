// Versions and ranges as npm writes them: 1.2.3-beta.1, ^1.2, ~1.2.3, >=1 <2, 1.x || 3.
#ifndef SEMVER_H
#define SEMVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// A version: 1.2.3, or 1.2.3-beta.1 with a pre-release. pre points into the text
// that the version was read from, which must outlive it, and is NULL if the version
// has no pre-release.
typedef struct {
    long long major, minor, patch;
    const char *pre;
    size_t pre_length;
} Version;

// Reads the length characters at text as a version, into *version. Returns false if
// they aren't one.
bool version_parse(const char *text, size_t length, Version *version);
// Negative if a comes before b, 0 if they are equal, and positive if after.
int version_compare(const Version *a, const Version *b);
void version_print(const Version *version, FILE *out);

// A range is opaque: only semver.c knows what is inside.
typedef struct Range Range;

// Reads text as a range, which keeps pointers into text. Returns NULL if it isn't
// one, and sets *error to a message saying why, which the caller must free. If
// there is no memory, it returns NULL and sets *error to NULL.
Range *range_parse(const char *text, char **error);
void range_free(Range *range);
bool range_allows(const Range *range, const Version *version);
// The highest of the count versions that the range allows, or NULL if it allows
// none of them.
const Version *range_highest(const Range *range, const Version *versions, size_t count);

#endif
