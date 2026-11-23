// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. getopt_long, from the C library on Linux and the BSDs, reads the options; the
// usage message, the exit status, and the checks of each value are ours.
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char USAGE[] = "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...\n";
static const char HELP[] =
    "\n"
    "Converts temperatures, such as 21.5c, -40f, or 300k.\n"
    "\n"
    "options:\n"
    "  -h, --help           show this help and exit\n"
    "  -t, --to UNIT        the unit to convert to: c, f, or k (default: c)\n"
    "  -d, --digits DIGITS  digits after the point, 0 to 9 (default: 1)\n";

static double to_celsius(double t, char unit) {
    switch (unit) {
    case 'f': return (t - 32) * 5 / 9;
    case 'k': return t - 273.15;
    default: return t;
    }
}

static double from_celsius(double t, char unit) {
    switch (unit) {
    case 'f': return t * 9 / 5 + 32;
    case 'k': return t + 273.15;
    default: return t;
    }
}

static int is_unit(const char *text) {
    return strlen(text) == 1 && strchr("cfk", text[0]) != NULL;
}

// Prints a message and the usage line to stderr, and exits with status 2.
static void usage_error(const char *message, const char *value) {
    fprintf(stderr, "temp: %s%s\n%s", message, value, USAGE);
    exit(2);
}

// Reads a number followed by c, f, or k into *value and *unit, and returns 1, or
// returns 0 if text isn't one.
static int parse_temperature(const char *text, double *value, char *unit) {
    size_t length = strlen(text);
    if (length < 2 || !is_unit(text + length - 1)) return 0;
    *unit = text[length - 1];
    char *end;
    errno = 0;
    *value = strtod(text, &end);
    // strtod stops at the first character that isn't part of the number, which must be
    // the unit.
    return end == text + length - 1 && errno == 0 && isfinite(*value);
}

int main(int argc, char **argv) {
    static const struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {"to", required_argument, NULL, 't'},
        {"digits", required_argument, NULL, 'd'},
        {NULL, 0, NULL, 0},
    };
    char to = 'c';
    int digits = 1;
    int option;
    // "ht:d:" lists the short options; a colon means that one takes a value. For an
    // option it doesn't know, or one without its value, getopt_long prints a message,
    // such as "unrecognized option '--bogus'", and returns '?'.
    while ((option = getopt_long(argc, argv, "ht:d:", options, NULL)) != -1) {
        switch (option) {
        case 'h':
            printf("%s%s", USAGE, HELP);
            return 0;
        case 't':
            if (!is_unit(optarg)) usage_error("unknown unit: ", optarg);
            to = optarg[0];
            break;
        case 'd':
            if (strlen(optarg) != 1 || optarg[0] < '0' || optarg[0] > '9') {
                usage_error("digits must be 0 to 9, not ", optarg);
            }
            digits = optarg[0] - '0';
            break;
        default:
            fputs(USAGE, stderr);
            return 2;
        }
    }
    // getopt_long has moved the arguments that aren't options to the end, from optind.
    if (optind == argc) usage_error("no temperatures", "");

    // Check every temperature before printing any, so that a mistake prints nothing.
    double value;
    char unit;
    for (int i = optind; i < argc; i++) {
        if (!parse_temperature(argv[i], &value, &unit)) {
            fprintf(stderr, "temp: not a temperature: %s\n", argv[i]);
            return 2;
        }
    }
    for (int i = optind; i < argc; i++) {
        parse_temperature(argv[i], &value, &unit);
        printf("%s = %.*f%c\n", argv[i], digits, from_celsius(to_celsius(value, unit), to), to);
    }
    return 0;
}
