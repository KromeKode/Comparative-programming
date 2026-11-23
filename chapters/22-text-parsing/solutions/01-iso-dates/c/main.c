// Copies stdin to stdout, with each date in it written as YYYY-MM-DD.
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const months[] = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December",
};

// POSIX regular expressions have no \b, no \d, and no (?: ), whose groups don't count. So
// the names come whole and then short, and the boundaries are checked by hand. A POSIX
// regex takes the longest match, so "January" wins over "Jan".
#define MONTH                                                                          \
    "(January|February|March|April|May|June|July|August|September|October|November|" \
    "December|Jan|Feb|Mar|Apr|Jun|Jul|Aug|Sep|Oct|Nov|Dec)"
#define D "([0-9]{1,2})"
#define Y "([0-9]{4})"
static const char DATE[] = D "/" D "/" Y "|" MONTH " " D ", " Y "|" D " " MONTH " " Y;
enum { GROUPS = 10 };  // the whole match, and nine groups

static bool is_word(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') ||
           c == '_';
}

static int days_in(int month, int year) {
    if (month == 2) return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) ? 29 : 28;
    return month == 4 || month == 6 || month == 9 || month == 11 ? 30 : 31;
}

static int number(const char *text, regmatch_t group) {
    return (int)strtol(text + group.rm_so, NULL, 10);  // it stops at the first non-digit
}

static int month_of(const char *text, regmatch_t group) {
    size_t length = (size_t)(group.rm_eo - group.rm_so);
    for (int i = 0; i < 12; i++) {
        if (strncmp(months[i], text + group.rm_so, length) == 0) return i + 1;
    }
    return 0;
}

// Writes the date that matched as YYYY-MM-DD, or as it was if it isn't a real date.
static void rewrite(const char *text, const regmatch_t *g) {
    // Only one of the three forms matched, and the groups of the other two are at -1.
    int day, month, year;
    if (g[1].rm_so != -1) {
        day = number(text, g[1]), month = number(text, g[2]), year = number(text, g[3]);
    } else if (g[4].rm_so != -1) {
        day = number(text, g[5]), month = month_of(text, g[4]), year = number(text, g[6]);
    } else {
        day = number(text, g[7]), month = month_of(text, g[8]), year = number(text, g[9]);
    }
    if (month < 1 || month > 12 || day < 1 || day > days_in(month, year)) {
        fwrite(text + g[0].rm_so, 1, (size_t)(g[0].rm_eo - g[0].rm_so), stdout);
    } else {
        printf("%04d-%02d-%02d", year, month, day);
    }
}

// Writes text, which ends with a '\0', with each date rewritten.
static void rewrite_all(const regex_t *date, const char *text) {
    size_t at = 0, written = 0;
    regmatch_t g[GROUPS];
    while (regexec(date, text + at, GROUPS, g, 0) == 0) {
        // regexec's offsets count from where the search began.
        for (int i = 0; i < GROUPS; i++) {
            if (g[i].rm_so != -1) g[i].rm_so += (regoff_t)at, g[i].rm_eo += (regoff_t)at;
        }
        size_t start = (size_t)g[0].rm_so, end = (size_t)g[0].rm_eo;
        if ((start > 0 && is_word(text[start - 1])) || is_word(text[end])) {
            at = start + 1;  // it touches a letter, a digit, or _, so look again just after
            continue;
        }
        fwrite(text + written, 1, start - written, stdout);
        rewrite(text, g);
        at = written = end;
    }
    fputs(text + written, stdout);
}

int main(void) {
    regex_t date;
    if (regcomp(&date, DATE, REG_EXTENDED) != 0) {
        fputs("isodates: bad regex\n", stderr);
        return 1;
    }
    char *text = NULL;
    size_t length = 0, capacity = 0, got;
    do {
        if (length == capacity) {
            capacity = capacity == 0 ? 65536 : 2 * capacity;
            text = realloc(text, capacity + 1);  // and a '\0'
            if (text == NULL) {
                fputs("isodates: out of memory\n", stderr);
                return 1;
            }
        }
        got = fread(text + length, 1, capacity - length, stdin);
        length += got;
    } while (got > 0);
    text[length] = '\0';
    // regexec reads up to a '\0', so a '\0' in the text ends one piece and starts another.
    for (size_t start = 0;; start++) {
        rewrite_all(&date, text + start);
        start += strlen(text + start);
        if (start == length) break;
        putchar('\0');
    }
    free(text);
    regfree(&date);
    return 0;
}
