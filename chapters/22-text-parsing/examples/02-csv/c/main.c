// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    RECORD_START,  // nothing of this record read yet
    FIELD_START,   // just after a comma
    UNQUOTED,      // in a field that doesn't start with a quote
    QUOTED,        // in a field that does
    QUOTE,         // just after a quote in a quoted field: its end, or the first of ""
    CR,            // just after a "\r" outside quotes, which needs a "\n" next
} State;

enum { END = -1 };  // stands for the end, so that it goes through the machine as well

typedef struct {
    char *data;
    size_t length, capacity;
} Buffer;

static void append(Buffer *b, const char *s, size_t n) {
    if (b->length + n > b->capacity) {
        while (b->length + n > b->capacity) b->capacity = b->capacity == 0 ? 64 : 2 * b->capacity;
        b->data = realloc(b->data, b->capacity);
        if (b->data == NULL) {
            fputs("csv2json: out of memory\n", stderr);
            exit(1);
        }
    }
    memcpy(b->data + b->length, s, n);
    b->length += n;
}

static void append_char(Buffer *b, char c) { append(b, &c, 1); }

// Ends a field: appends it to the record, which is kept as JSON text, after a comma
// unless it's the first. Writes it as JavaScript's JSON.stringify does. Bytes of UTF-8
// above 0x7F are written as they are.
static void end_field(Buffer *record, Buffer *field, size_t *count) {
    if (*count > 0) append_char(record, ',');
    append_char(record, '"');
    for (size_t i = 0; i < field->length; i++) {
        unsigned char c = (unsigned char)field->data[i];
        switch (c) {
            case '"': append(record, "\\\"", 2); break;
            case '\\': append(record, "\\\\", 2); break;
            case '\b': append(record, "\\b", 2); break;
            case '\f': append(record, "\\f", 2); break;
            case '\n': append(record, "\\n", 2); break;
            case '\r': append(record, "\\r", 2); break;
            case '\t': append(record, "\\t", 2); break;
            default:
                if (c < 0x20) {
                    char escape[7];
                    snprintf(escape, sizeof escape, "\\u%04x", c);
                    append(record, escape, 6);
                } else {
                    append_char(record, (char)c);
                }
        }
    }
    append_char(record, '"');
    field->length = 0;
    (*count)++;
}

// Prints a message for the error at line and column, and returns 1, the exit status.
static int error(int line, int column, const char *format, ...) {
    fprintf(stderr, "csv2json: line %d, column %d: ", line, column);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    return 1;
}

// Prints each record in text, and returns 0, or 1 at the first error.
static int parse(const char *text, size_t size) {
    State state = RECORD_START;
    Buffer record = {0}, field = {0};
    size_t count = 0;  // fields in the record so far
    size_t width = 0;  // how many fields the first record has, once it has ended
    int line = 1, column = 0;  // where c is: columns count characters, from 1
    int start_line = 0, start_column = 0;  // where a quoted field's quote is, or the "\r"
    int record_line = 1;
    int status = 0;
    for (size_t i = 0; i <= size && status == 0; i++) {
        int c = i < size ? (unsigned char)text[i] : END;
        // The text is UTF-8, so a character is one to four bytes. All but the first are
        // 10xxxxxx, and they don't move the column. None of them is a quote, a comma, or
        // a line end, so they go into the field with the rest of their character.
        if ((c & 0xC0) != 0x80) column++;
        if (state == RECORD_START) record_line = line;
        int ended = 0;  // whether c ends a record
        if (state == QUOTED) {
            if (c == END) {
                status = error(start_line, start_column, "quoted field is never closed");
            } else if (c == '"') {
                state = QUOTE;
            } else {
                append_char(&field, (char)c);
            }
        } else if (state == QUOTE && c == '"') {
            append_char(&field, '"');
            state = QUOTED;
        } else if (state == CR) {
            if (c == '\n') {
                ended = 1;
            } else {
                status = error(start_line, start_column,
                               "a carriage return without a line feed after it");
            }
        } else if (state == RECORD_START && c == END) {
            // the end, at the start or just after a line end: no record
        } else if (c == '"' && state != QUOTE) {
            if (state == UNQUOTED) {
                status = error(line, column, "a quote in an unquoted field");
            } else {
                start_line = line;
                start_column = column;
                state = QUOTED;
            }
        } else if (c == ',') {
            end_field(&record, &field, &count);
            state = FIELD_START;
        } else if (c == '\n' || c == END) {
            ended = 1;
        } else if (c == '\r') {
            start_line = line;
            start_column = column;
            state = CR;
        } else if (state == QUOTE) {
            status = error(line, column, "expected a comma or a line end after a closing quote");
        } else {
            append_char(&field, (char)c);
            state = UNQUOTED;
        }
        if (ended) {
            end_field(&record, &field, &count);
            if (width == 0) width = count;
            if (count != width) {
                status = error(record_line, 1, "record has %zu field%s, but the first has %zu",
                               count, count == 1 ? "" : "s", width);
            } else {
                printf("[%.*s]\n", (int)record.length, record.data);
            }
            record.length = 0;
            count = 0;
            state = RECORD_START;
        }
        if (c == '\n') {
            line++;
            column = 0;
        }
    }
    free(record.data);
    free(field.data);
    return status;
}

int main(void) {
    Buffer input = {0};
    char chunk[65536];
    size_t got;
    while ((got = fread(chunk, 1, sizeof chunk, stdin)) > 0) append(&input, chunk, got);
    if (ferror(stdin)) {
        perror("csv2json");
        return 1;
    }
    int status = parse(input.data, input.length);
    free(input.data);
    return status;
}
