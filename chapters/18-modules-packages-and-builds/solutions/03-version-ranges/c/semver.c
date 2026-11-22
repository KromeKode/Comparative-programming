// Versions and ranges. Every function and type here that semver.h doesn't declare
// is static, so other files can't see it, and its name can't clash with theirs.
#include "semver.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef enum { EQ, LT, LE, GT, GE } Op;

// A simple comparator: an operator and a full version.
typedef struct {
    Op op;
    Version bound;
} Simple;

// A set of simple comparators, all of which a version must pass.
typedef struct {
    Simple *simples;
    size_t count;
} Set;

struct Range {
    Set *sets;
    size_t count;
};

static bool is_digit(char c) { return c >= '0' && c <= '9'; }

static bool all_digits(const char *text, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if (!is_digit(text[i])) return false;
    }
    return true;
}

// A number in a version: 0, or up to 15 digits that don't start with 0.
static bool parse_number(const char *text, size_t length, long long *number) {
    if (length == 0 || length > 15 || !all_digits(text, length)) return false;
    if (length > 1 && text[0] == '0') return false;
    *number = 0;
    for (size_t i = 0; i < length; i++) *number = *number * 10 + (text[i] - '0');
    return true;
}

// A pre-release is identifiers separated by dots. Each is digits without a 0 in
// front, or letters, digits, and hyphens with at least one that isn't a digit.
static bool valid_pre(const char *text, size_t length) {
    size_t start = 0;
    for (size_t i = 0; i <= length; i++) {
        if (i < length && text[i] != '.') {
            char c = text[i];
            if (!is_digit(c) && !(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') && c != '-') {
                return false;
            }
            continue;
        }
        size_t size = i - start;
        if (size == 0) return false;
        if (size > 1 && text[start] == '0' && all_digits(text + start, size)) return false;
        start = i + 1;
    }
    return true;
}

// Splits text at its first '-': the part before is the core, and the part after
// is the pre-release, if there is a '-'.
static size_t core_length(const char *text, size_t length) {
    const char *dash = memchr(text, '-', length);
    return dash ? (size_t)(dash - text) : length;
}

bool version_parse(const char *text, size_t length, Version *version) {
    size_t core = core_length(text, length);
    long long *parts[] = {&version->major, &version->minor, &version->patch};
    size_t start = 0;
    for (int i = 0; i < 3; i++) {
        const char *dot = memchr(text + start, '.', core - start);
        size_t end = i < 2 ? (dot ? (size_t)(dot - text) : core) : core;
        if (i < 2 && dot == NULL) return false;
        if (!parse_number(text + start, end - start, parts[i])) return false;
        start = end + 1;
    }
    version->pre = NULL;
    version->pre_length = 0;
    if (core < length) {
        if (!valid_pre(text + core + 1, length - core - 1)) return false;
        version->pre = text + core + 1;
        version->pre_length = length - core - 1;
    }
    return true;
}

// The length of the identifier at text, which ends at a '.' or after length.
static size_t identifier_length(const char *text, size_t length) {
    const char *dot = memchr(text, '.', length);
    return dot ? (size_t)(dot - text) : length;
}

// Numbers compare as numbers, and come before other identifiers, which compare
// character by character. Numbers have no 0 in front, so a longer one is larger.
static int compare_identifiers(const char *a, size_t a_length, const char *b, size_t b_length) {
    bool a_number = all_digits(a, a_length), b_number = all_digits(b, b_length);
    if (a_number != b_number) return a_number ? -1 : 1;
    if (a_number && a_length != b_length) return a_length < b_length ? -1 : 1;
    int order = memcmp(a, b, a_length < b_length ? a_length : b_length);
    if (order != 0) return order;
    return (a_length > b_length) - (a_length < b_length);
}

static int compare_core(const Version *a, const Version *b) {
    if (a->major != b->major) return a->major < b->major ? -1 : 1;
    if (a->minor != b->minor) return a->minor < b->minor ? -1 : 1;
    if (a->patch != b->patch) return a->patch < b->patch ? -1 : 1;
    return 0;
}

int version_compare(const Version *a, const Version *b) {
    int order = compare_core(a, b);
    if (order != 0) return order;
    // A version without a pre-release comes after the same version with one.
    if (a->pre == NULL || b->pre == NULL) return (a->pre == NULL) - (b->pre == NULL);
    const char *x = a->pre, *y = b->pre;
    size_t x_left = a->pre_length, y_left = b->pre_length;
    while (x_left > 0 && y_left > 0) {
        size_t x_size = identifier_length(x, x_left), y_size = identifier_length(y, y_left);
        order = compare_identifiers(x, x_size, y, y_size);
        if (order != 0) return order;
        // Step past the identifier and its dot, if it has one.
        x += x_size < x_left ? x_size + 1 : x_size;
        x_left -= x_size < x_left ? x_size + 1 : x_size;
        y += y_size < y_left ? y_size + 1 : y_size;
        y_left -= y_size < y_left ? y_size + 1 : y_size;
    }
    // The one with identifiers left over comes after.
    return (x_left > 0) - (y_left > 0);
}

void version_print(const Version *version, FILE *out) {
    fprintf(out, "%lld.%lld.%lld", version->major, version->minor, version->patch);
    if (version->pre != NULL) fprintf(out, "-%.*s", (int)version->pre_length, version->pre);
}

static bool is_wild(const char *text, size_t length) {
    return length == 1 && (text[0] == 'x' || text[0] == 'X' || text[0] == '*');
}

// The first version after all those whose parts up to level match: the -0
// pre-release of the next major, minor, or patch.
static Version after(const long long nums[3], int level) {
    Version version = {nums[0], nums[1], nums[2], "0", 1};
    if (level == 0) version = (Version){nums[0] + 1, 0, 0, "0", 1};
    if (level == 1) version = (Version){nums[0], nums[1] + 1, 0, "0", 1};
    if (level == 2) version = (Version){nums[0], nums[1], nums[2] + 1, "0", 1};
    return version;
}

// Writes the simple comparators that the comparator at text stands for to out, and
// returns how many there are, from 0 to 2, or -1 if it isn't a comparator.
static int expand(const char *text, size_t length, Simple out[2]) {
    static const struct {
        const char *text;
        char op;
    } operators[] = {{"<=", 'l'}, {">=", 'g'}, {"<", '<'}, {">", '>'},
                     {"=", '='},  {"^", '^'},  {"~", '~'}};
    char op = ' ';
    for (size_t i = 0; i < sizeof operators / sizeof operators[0]; i++) {
        size_t size = strlen(operators[i].text);
        if (length >= size && memcmp(text, operators[i].text, size) == 0) {
            op = operators[i].op;
            text += size;
            length -= size;
            break;
        }
    }
    size_t core = core_length(text, length);
    // Up to three parts, separated by dots. known counts the numbers, which must
    // come before any wildcards.
    long long nums[3] = {0, 0, 0};
    int parts = 0, known = 0;
    size_t start = 0;
    while (true) {
        if (parts == 3) return -1;
        const char *dot = memchr(text + start, '.', core - start);
        size_t end = dot ? (size_t)(dot - text) : core;
        if (is_wild(text + start, end - start)) {
            // A wildcard, so every part after it must be one too.
        } else if (known == parts && parse_number(text + start, end - start, &nums[known])) {
            known++;
        } else {
            return -1;
        }
        parts++;
        if (dot == NULL) break;
        start = end + 1;
    }
    Version low = {nums[0], nums[1], nums[2], NULL, 0};
    if (core < length) {
        // A pre-release can follow only three numbers.
        if (known < 3 || !valid_pre(text + core + 1, length - core - 1)) return -1;
        low.pre = text + core + 1;
        low.pre_length = length - core - 1;
    }
    Version first = {0, 0, 0, "0", 1};  // before every version
    if (known == 0) {
        // Any version: nothing is outside it, and nothing is above or below it.
        if (op != '<' && op != '>') return 0;
        out[0] = (Simple){LT, first};
        return 1;
    }
    Version next = after(nums, known - 1);
    switch (op) {
        case ' ':
        case '=':
            if (known == 3) {
                out[0] = (Simple){EQ, low};
                return 1;
            }
            out[0] = (Simple){GE, low};
            out[1] = (Simple){LT, next};
            return 2;
        case '>':
            next.pre = NULL;
            next.pre_length = 0;
            out[0] = known == 3 ? (Simple){GT, low} : (Simple){GE, next};
            return 1;
        case 'g':
            out[0] = (Simple){GE, low};
            return 1;
        case '<':
            if (known < 3) low = (Version){nums[0], nums[1], nums[2], "0", 1};
            out[0] = (Simple){LT, low};
            return 1;
        case 'l':
            out[0] = known == 3 ? (Simple){LE, low} : (Simple){LT, next};
            return 1;
        case '~':
            out[0] = (Simple){GE, low};
            out[1] = (Simple){LT, after(nums, known == 1 ? 0 : 1)};
            return 2;
        default: {
            // ^ allows changes that don't change the first part that isn't 0.
            int level = known - 1;
            for (int i = known - 1; i >= 0; i--) {
                if (nums[i] != 0) level = i;
            }
            out[0] = (Simple){GE, low};
            out[1] = (Simple){LT, after(nums, level)};
            return 2;
        }
    }
}

static bool holds(const Version *version, const Simple *simple) {
    int order = version_compare(version, &simple->bound);
    switch (simple->op) {
        case EQ: return order == 0;
        case LT: return order < 0;
        case LE: return order <= 0;
        case GT: return order > 0;
        default: return order >= 0;
    }
}

bool range_allows(const Range *range, const Version *version) {
    for (size_t i = 0; i < range->count; i++) {
        const Set *set = &range->sets[i];
        bool passes = true, named = false;
        for (size_t j = 0; j < set->count; j++) {
            const Version *bound = &set->simples[j].bound;
            if (!holds(version, &set->simples[j])) passes = false;
            if (bound->pre != NULL && compare_core(bound, version) == 0) named = true;
        }
        // A pre-release is in only if a comparator of the set names a pre-release of
        // the same major, minor, and patch.
        if (passes && (version->pre == NULL || named)) return true;
    }
    return false;
}

const Version *range_highest(const Range *range, const Version *versions, size_t count) {
    const Version *best = NULL;
    for (size_t i = 0; i < count; i++) {
        if (range_allows(range, &versions[i]) &&
            (best == NULL || version_compare(&versions[i], best) > 0)) {
            best = &versions[i];
        }
    }
    return best;
}

void range_free(Range *range) {
    if (range == NULL) return;
    for (size_t i = 0; i < range->count; i++) free(range->sets[i].simples);
    free(range->sets);
    free(range);
}

// A message made like printf's output, which the caller frees, or NULL if there is
// no memory for it.
static char *message(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int size = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *text = malloc((size_t)size + 1);
    if (text == NULL) return NULL;
    va_start(args, format);
    vsnprintf(text, (size_t)size + 1, format, args);
    va_end(args);
    return text;
}

static bool is_space(char c) { return c == ' ' || (c >= '\t' && c <= '\r'); }

Range *range_parse(const char *text, char **error) {
    *error = NULL;
    Range *range = calloc(1, sizeof(Range));
    if (range == NULL) return NULL;
    const char *side = text;
    while (true) {
        const char *bars = strstr(side, "||");
        size_t side_length = bars ? (size_t)(bars - side) : strlen(side);
        Set *sets = realloc(range->sets, (range->count + 1) * sizeof(Set));
        if (sets == NULL) goto fail;
        range->sets = sets;
        Set *set = &range->sets[range->count++];
        *set = (Set){NULL, 0};
        // A wildcard stands for no simple comparators at all, so count the
        // comparators as written, not the simple ones.
        int tokens = 0;
        for (size_t i = 0; i < side_length;) {
            if (is_space(side[i])) {
                i++;
                continue;
            }
            size_t end = i;
            while (end < side_length && !is_space(side[end])) end++;
            Simple simples[2];
            int count = expand(side + i, end - i, simples);
            if (count < 0) {
                *error = message("%.*s isn't a comparator", (int)(end - i), side + i);
                goto fail;
            }
            Simple *grown = realloc(set->simples, (set->count + 2) * sizeof(Simple));
            if (grown == NULL) goto fail;
            set->simples = grown;
            for (int k = 0; k < count; k++) set->simples[set->count++] = simples[k];
            tokens++;
            i = end;
        }
        if (tokens == 0) {
            *error = message("each side of || needs a comparator");
            goto fail;
        }
        if (bars == NULL) break;
        side = bars + 2;
    }
    return range;
fail:
    range_free(range);
    return NULL;
}
