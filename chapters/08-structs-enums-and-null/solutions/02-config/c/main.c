#include <stdio.h>
#include <string.h>

// C has no optional type, so each setting needs a value that means "not set". An empty
// host is not allowed, and 0 is not a valid port or timeout, so those can do the job.
typedef struct {
    char host[256];
    long port;
    long timeout;
} Config;

// Removes spaces, tabs, and line endings from both ends, in place.
static char *trim(char *text) {
    while (*text == ' ' || *text == '\t') {
        text++;
    }
    size_t length = strlen(text);
    while (length > 0 && strchr(" \t\r\n", text[length - 1]) != NULL) {
        text[--length] = '\0';
    }
    return text;
}

// Returns the number if text is digits only and from 1 to high, or else 0.
static long parse_number(const char *text, long high) {
    if (*text == '\0') {
        return 0;
    }
    long value = 0;
    for (const char *p = text; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
        value = value * 10 + (*p - '0');
        if (value > high) {
            return 0; // stopping here also keeps value from overflowing
        }
    }
    return value; // 0 for "0" or "000", which is not allowed either
}

// Sets *setting from value, or reports a bad value and leaves it alone.
static void set_number(long *setting, const char *key, const char *value, long high,
                       int number) {
    long parsed = parse_number(value, high);
    if (*value == '\0') {
        *setting = 0;
    } else if (parsed == 0) {
        printf("line %d: bad %s \"%s\"\n", number, key, value);
    } else {
        *setting = parsed;
    }
}

int main(void) {
    Config config = {.host = "", .port = 0, .timeout = 0};
    static char buffer[1 << 16];
    int number = 0;
    while (fgets(buffer, sizeof buffer, stdin) != NULL) {
        number++;
        char *line = trim(buffer);
        if (*line == '\0' || *line == '#') {
            continue;
        }
        char *equals = strchr(line, '=');
        if (equals == NULL) {
            printf("line %d: expected key = value\n", number);
            continue;
        }
        *equals = '\0';
        char *key = trim(line);
        char *value = trim(equals + 1);
        if (strcmp(key, "host") == 0) {
            snprintf(config.host, sizeof config.host, "%s", value);
        } else if (strcmp(key, "port") == 0) {
            set_number(&config.port, key, value, 65535, number);
        } else if (strcmp(key, "timeout") == 0) {
            set_number(&config.timeout, key, value, 86400, number);
        } else {
            printf("line %d: unknown key \"%s\"\n", number, key);
        }
    }

    printf("host: %s\n", config.host[0] != '\0' ? config.host : "localhost (default)");
    if (config.port != 0) {
        printf("port: %ld\n", config.port);
    } else {
        printf("port: 8080 (default)\n");
    }
    if (config.timeout != 0) {
        printf("timeout: %lds\n", config.timeout);
    } else {
        printf("timeout: none\n");
    }
    return 0;
}
