#include "duration.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static const char UNITS[] = "dhms";  // largest first, which is the order they come in
static const long long SECONDS[] = {86400, 3600, 60, 1};  // in each of the units

bool parse_duration(const char *text, long long *seconds) {
    if (*text == '\0') return false;
    long long total = 0;
    size_t next_unit = 0;  // each part's unit comes after the one before it in UNITS
    const char *p = text;
    while (*p != '\0') {
        const char *start = p;
        long long number = 0;
        while ('0' <= *p && *p <= '9') {
            if (number > (LLONG_MAX - (*p - '0')) / 10) return false;
            number = 10 * number + (*p - '0');
            p++;
        }
        if (p == start) return false;  // a part with no number
        if (*p == '\0') return false;  // a number with no unit
        const char *found = strchr(UNITS, *p);
        if (found == NULL) return false;  // not a unit
        size_t unit = (size_t)(found - UNITS);
        if (unit < next_unit) return false;  // out of order, or twice
        if (number > (LLONG_MAX - total) / SECONDS[unit]) return false;
        total += number * SECONDS[unit];
        next_unit = unit + 1;
        p++;
    }
    *seconds = total;
    return true;
}

bool format_duration(long long seconds, char *out, size_t size) {
    if (seconds < 0) return false;
    char text[32] = "";  // room for LLONG_MAX seconds, which is 106751991167300d15h30m7s
    size_t length = 0;
    for (size_t unit = 0; unit < 4; unit++) {
        long long amount = seconds / SECONDS[unit];
        seconds %= SECONDS[unit];
        if (amount > 0) {
            int n = snprintf(text + length, sizeof text - length, "%lld%c", amount, UNITS[unit]);
            length += (size_t)n;
        }
    }
    if (length == 0) strcpy(text, "0s");
    int n = snprintf(out, size, "%s", text);
    return n >= 0 && (size_t)n < size;
}
