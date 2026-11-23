// Reads one JSON text on stdin, strictly as RFC 8259 has it, and prints it compactly:
// no spaces, keys in the order they came, numbers as they were written, and strings
// written again with only the escapes they need. Or prints the first error's line and
// column, and exits with 1.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_DEPTH = 1000 };

typedef struct {
    char *data;
    size_t length, capacity;
} Buffer;

static void append(Buffer *b, const char *s, size_t n) {
    if (b->length + n + 1 > b->capacity) {
        while (b->length + n + 1 > b->capacity) b->capacity = b->capacity ? 2 * b->capacity : 64;
        b->data = realloc(b->data, b->capacity);
        if (b->data == NULL) {
            fputs("jsoncheck: out of memory\n", stderr);
            exit(1);
        }
    }
    memcpy(b->data + b->length, s, n);
    b->length += n;
    b->data[b->length] = '\0';  // so that data is a C string too
}

static void append_str(Buffer *b, const char *s) { append(b, s, strlen(s)); }

// Appends s, which is UTF-8, as a JSON string, with only the escapes it needs. The bytes
// of characters above U+007F are written as they are.
static void quote(Buffer *out, const char *s, size_t n) {
    append_str(out, "\"");
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        switch (c) {
            case '"': append_str(out, "\\\""); break;
            case '\\': append_str(out, "\\\\"); break;
            case '\b': append_str(out, "\\b"); break;
            case '\f': append_str(out, "\\f"); break;
            case '\n': append_str(out, "\\n"); break;
            case '\r': append_str(out, "\\r"); break;
            case '\t': append_str(out, "\\t"); break;
            default:
                if (c < 0x20) {
                    char escape[7];
                    snprintf(escape, sizeof escape, "\\u%04x", c);
                    append_str(out, escape);
                } else {
                    append(out, (const char *)&c, 1);
                }
        }
    }
    append_str(out, "\"");
}

// Appends the code point as UTF-8: one to four bytes.
static void append_utf8(Buffer *out, uint32_t c) {
    char bytes[4];
    size_t n;
    if (c < 0x80) {
        bytes[0] = (char)c, n = 1;
    } else if (c < 0x800) {
        bytes[0] = (char)(0xC0 | c >> 6), bytes[1] = (char)(0x80 | (c & 0x3F)), n = 2;
    } else if (c < 0x10000) {
        bytes[0] = (char)(0xE0 | c >> 12), bytes[1] = (char)(0x80 | (c >> 6 & 0x3F));
        bytes[2] = (char)(0x80 | (c & 0x3F)), n = 3;
    } else {
        bytes[0] = (char)(0xF0 | c >> 18), bytes[1] = (char)(0x80 | (c >> 12 & 0x3F));
        bytes[2] = (char)(0x80 | (c >> 6 & 0x3F)), bytes[3] = (char)(0x80 | (c & 0x3F)), n = 4;
    }
    append(out, bytes, n);
}

typedef struct {
    const char *text;  // UTF-8, so a character is one to four bytes
    size_t length, i;
    int depth;
    size_t error_at;  // a byte offset into the text
    char message[128];
} Parser;

// The byte at i, or -1 at the end.
static int peek(const Parser *p) { return p->i < p->length ? (unsigned char)p->text[p->i] : -1; }

// Each parsing function returns false after one of these has filled in the error.
static bool error_at(Parser *p, size_t at, const char *message) {
    p->error_at = at;
    snprintf(p->message, sizeof p->message, "%s", message);
    return false;
}

static bool fail(Parser *p, const char *expected) {
    Buffer found = {0};
    if (p->i < p->length) {
        // The whole character: its first byte, and the 10xxxxxx bytes after it.
        size_t n = 1;
        while (p->i + n < p->length && ((unsigned char)p->text[p->i + n] & 0xC0) == 0x80) n++;
        quote(&found, p->text + p->i, n);
    } else {
        append_str(&found, "the end of the input");
    }
    p->error_at = p->i;
    snprintf(p->message, sizeof p->message, "expected %s, found %s", expected, found.data);
    free(found.data);
    return false;
}

static void space(Parser *p) {
    while (peek(p) == ' ' || peek(p) == '\t' || peek(p) == '\n' || peek(p) == '\r') p->i++;
}

static bool starts_with(const Parser *p, size_t at, const char *word) {
    size_t n = strlen(word);
    return at + n <= p->length && memcmp(p->text + at, word, n) == 0;
}

static bool is_digit(const Parser *p) { return peek(p) >= '0' && peek(p) <= '9'; }

static bool value(Parser *p, Buffer *out);

// The four hex digits at text[at], or -1 if they aren't.
static long hex4(const Parser *p, size_t at) {
    if (at + 4 > p->length) return -1;
    long n = 0;
    for (size_t k = at; k < at + 4; k++) {
        char c = p->text[k];
        if (c >= '0' && c <= '9') n = n * 16 + (c - '0');
        else if (c >= 'a' && c <= 'f') n = n * 16 + (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') n = n * 16 + (c - 'A' + 10);
        else return -1;
    }
    return n;
}

// Appends the string at i to out, with its escapes undone, as UTF-8.
static bool string(Parser *p, Buffer *out) {
    size_t start = p->i;
    p->i++;  // the "
    for (;;) {
        int c = peek(p);
        if (c == -1) return error_at(p, start, "string is never closed");
        if (c == '"') {
            p->i++;
            return true;
        }
        if (c < 0x20) return error_at(p, p->i, "control character in a string");
        if (c != '\\') {
            append(out, p->text + p->i, 1);  // a byte of UTF-8, which goes as it is
            p->i++;
            continue;
        }
        if (p->i + 1 == p->length) return error_at(p, start, "string is never closed");
        size_t at = p->i;
        char escape = p->text[p->i + 1];
        p->i += 2;
        const char *simple = strchr("\"\\/bfnrt", escape);
        if (escape != '\0' && simple != NULL) {
            append(out, &"\"\\/\b\f\n\r\t"[simple - "\"\\/bfnrt"], 1);
            continue;
        }
        long code = escape == 'u' ? hex4(p, p->i) : -1;
        if (code == -1) return error_at(p, at, "invalid escape");
        p->i += 4;
        if (code >= 0xDC00 && code <= 0xDFFF) return error_at(p, at, "unpaired surrogate");
        if (code >= 0xD800 && code <= 0xDBFF) {
            // A character above U+FFFF is two escapes, a high surrogate and a low one.
            long low = starts_with(p, p->i, "\\u") ? hex4(p, p->i + 2) : -1;
            if (low < 0xDC00 || low > 0xDFFF) return error_at(p, at, "unpaired surrogate");
            code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
            p->i += 6;
        }
        append_utf8(out, (uint32_t)code);
    }
}

static bool digits(Parser *p) {
    if (!is_digit(p)) return fail(p, "a digit");
    while (is_digit(p)) p->i++;
    return true;
}

static bool number(Parser *p, Buffer *out) {
    size_t start = p->i;
    if (peek(p) == '-') p->i++;
    if (peek(p) == '0') {
        p->i++;  // a 0 can't have digits after it
    } else if (!digits(p)) {
        return false;
    }
    if (peek(p) == '.') {
        p->i++;
        if (!digits(p)) return false;
    }
    if (peek(p) == 'e' || peek(p) == 'E') {
        p->i++;
        if (peek(p) == '+' || peek(p) == '-') p->i++;
        if (!digits(p)) return false;
    }
    append(out, p->text + start, p->i - start);  // as it was written
    return true;
}

// The keys of one object so far. C has no set, so this is a list, searched from the
// start: fine for objects of a few hundred keys.
typedef struct {
    Buffer *keys;
    size_t count, capacity;
} Keys;

static bool add_key(Keys *k, Buffer key) {
    for (size_t j = 0; j < k->count; j++) {
        if (k->keys[j].length == key.length && memcmp(k->keys[j].data, key.data, key.length) == 0) {
            return false;
        }
    }
    if (k->count == k->capacity) {
        k->capacity = k->capacity ? 2 * k->capacity : 8;
        k->keys = realloc(k->keys, k->capacity * sizeof *k->keys);
        if (k->keys == NULL) exit(1);
    }
    k->keys[k->count++] = key;
    return true;
}

static void free_keys(Keys *k) {
    for (size_t j = 0; j < k->count; j++) free(k->keys[j].data);
    free(k->keys);
}

static bool object(Parser *p, Buffer *out) {
    p->i++;  // the {
    space(p);
    append_str(out, "{");
    Keys keys = {0};
    bool ok = true;
    if (peek(p) == '}') {
        p->i++;
    } else {
        for (;;) {
            if (peek(p) != '"') {
                ok = fail(p, keys.count == 0 ? "a string key or \"}\"" : "a string key");
                break;
            }
            size_t at = p->i;
            Buffer key = {0};
            append_str(&key, "");  // so that an empty key has data too
            if (!string(p, &key)) {
                free(key.data);
                ok = false;
                break;
            }
            if (!add_key(&keys, key)) {
                Buffer quoted = {0};
                quote(&quoted, key.data, key.length);
                snprintf(p->message, sizeof p->message, "duplicate key %s", quoted.data);
                p->error_at = at;
                free(quoted.data);
                free(key.data);
                ok = false;
                break;
            }
            space(p);
            if (peek(p) != ':') {
                ok = fail(p, "\":\"");
                break;
            }
            p->i++;
            space(p);
            quote(out, key.data, key.length);
            append_str(out, ":");
            if (!value(p, out)) {
                ok = false;
                break;
            }
            space(p);
            if (peek(p) == '}') {
                p->i++;
                break;
            }
            if (peek(p) != ',') {
                ok = fail(p, "\",\" or \"}\"");
                break;
            }
            append_str(out, ",");
            p->i++;
            space(p);
        }
    }
    free_keys(&keys);
    append_str(out, "}");
    return ok;
}

static bool array(Parser *p, Buffer *out) {
    p->i++;  // the [
    space(p);
    append_str(out, "[");
    if (peek(p) == ']') {
        p->i++;
        append_str(out, "]");
        return true;
    }
    for (;;) {
        if (!value(p, out)) return false;
        space(p);
        if (peek(p) == ']') {
            p->i++;
            append_str(out, "]");
            return true;
        }
        if (peek(p) != ',') return fail(p, "\",\" or \"]\"");
        append_str(out, ",");
        p->i++;
        space(p);
    }
}

static bool value(Parser *p, Buffer *out) {
    int c = peek(p);
    if (c == '{' || c == '[') {
        if (++p->depth > MAX_DEPTH) return error_at(p, p->i, "nesting deeper than 1000");
        bool ok = c == '{' ? object(p, out) : array(p, out);
        p->depth--;
        return ok;
    }
    if (c == '"') {
        Buffer s = {0};
        bool ok = string(p, &s);
        if (ok) quote(out, s.data, s.length);
        free(s.data);
        return ok;
    }
    if (c == '-' || is_digit(p)) return number(p, out);
    const char *words[] = {"true", "false", "null"};
    for (int k = 0; k < 3; k++) {
        if (starts_with(p, p->i, words[k])) {
            p->i += strlen(words[k]);
            append_str(out, words[k]);
            return true;
        }
    }
    return fail(p, "a value");
}

int main(void) {
    Buffer input = {0};
    char chunk[65536];
    size_t got;
    while ((got = fread(chunk, 1, sizeof chunk, stdin)) > 0) append(&input, chunk, got);
    Parser p = {.text = input.data, .length = input.length};
    Buffer out = {0};
    space(&p);
    bool ok = value(&p, &out);
    if (ok) {
        space(&p);
        if (p.i < p.length) ok = fail(&p, "the end of the input");
    }
    int status = 0;
    if (ok) {
        printf("%s\n", out.data);
    } else {
        // Lines end at "\n", and columns count characters from 1: each byte that isn't a
        // 10xxxxxx byte starts a character.
        int line = 1, column = 1;
        for (size_t i = 0; i < p.error_at; i++) {
            if (input.data[i] == '\n') {
                line++;
                column = 1;
            } else if (((unsigned char)input.data[i] & 0xC0) != 0x80) {
                column++;
            }
        }
        fprintf(stderr, "jsoncheck: line %d, column %d: %s\n", line, column, p.message);
        status = 1;
    }
    free(input.data);
    free(out.data);
    return status;
}
