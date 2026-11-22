#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_MACROS 256
#define MAX_PARAMS 32

struct macro {
    char *name;
    int param_count;  // -1 for a macro that takes no parentheses
    char *params[MAX_PARAMS];
    char *body;
};

static struct macro macros[MAX_MACROS];
static int macro_count;

// What is wrong with the line, set by the function that finds the problem.
static char error[2 * MAX_LINE];

// A piece of a longer string, so it doesn't end in '\0'.
struct slice {
    const char *text;
    size_t length;
};

// A string that grows as text is added to its end.
struct buffer {
    char *text;
    size_t length, capacity;
};

static struct buffer new_buffer(void) {
    struct buffer b = {malloc(64), 0, 64};
    if (b.text == NULL) abort();
    b.text[0] = '\0';
    return b;
}

static void add(struct buffer *b, const char *text, size_t length) {
    if (b->length + length >= b->capacity) {
        b->capacity = 2 * (b->length + length + 1);
        b->text = realloc(b->text, b->capacity);
        if (b->text == NULL) abort();
    }
    memcpy(b->text + b->length, text, length);
    b->length += length;
    b->text[b->length] = '\0';
}

// The macros whose expansions we are inside, innermost first. Each link lives
// on the stack of the call that expands its macro, so it goes away with it.
struct disabled {
    const struct macro *macro;
    const struct disabled *next;
};

static bool is_disabled(const struct disabled *link, const struct macro *macro) {
    for (; link != NULL; link = link->next) {
        if (link->macro == macro) return true;
    }
    return false;
}

static bool is_word_char(char c) { return isalnum((unsigned char)c) || c == '_'; }

static bool equal(struct slice a, struct slice b) {
    return a.length == b.length && memcmp(a.text, b.text, a.length) == 0;
}

static bool same(struct slice word, const char *name) {
    return equal(word, (struct slice){name, strlen(name)});
}

static struct slice trim(const char *text, size_t length) {
    while (length > 0 && isspace((unsigned char)text[0])) text++, length--;
    while (length > 0 && isspace((unsigned char)text[length - 1])) length--;
    return (struct slice){text, length};
}

static char *copy(struct slice s) {
    char *text = malloc(s.length + 1);
    if (text == NULL) abort();
    memcpy(text, s.text, s.length);
    text[s.length] = '\0';
    return text;
}

static size_t word_end(const char *text, size_t length, size_t start) {
    while (start < length && is_word_char(text[start])) start++;
    return start;
}

// The index just past the string that starts at text[start], a quote.
static size_t string_end(const char *text, size_t length, size_t start) {
    const char *close = memchr(text + start + 1, '"', length - start - 1);
    return close == NULL ? length : (size_t)(close - text) + 1;
}

static bool is_name(struct slice word) {
    return word.length > 0 && !isdigit((unsigned char)word.text[0]) &&
           word_end(word.text, word.length, 0) == word.length;
}

static struct macro *find_macro(struct slice word) {
    for (int i = 0; i < macro_count; i++) {
        if (same(word, macros[i].name)) return &macros[i];
    }
    return NULL;
}

static int find_param(const struct macro *macro, struct slice word) {
    for (int i = 0; i < macro->param_count; i++) {
        if (same(word, macro->params[i])) return i;
    }
    return -1;
}

// Splits the arguments of a call at its top-level commas, and trims each one.
// Returns how many there are, and sets *after to the index past the ")", or
// returns -1 for a call with no ")".
static int split_args(const char *text, size_t length, size_t open, struct slice *args,
                      size_t *after) {
    int count = 0, depth = 0;
    size_t start = open + 1, i = open + 1;
    while (i < length) {
        char c = text[i];
        if (c == '"') {
            i = string_end(text, length, i);
            continue;
        }
        if (c == '(') {
            depth++;
        } else if (c == ')' && depth > 0) {
            depth--;
        } else if (c == ')' || (c == ',' && depth == 0)) {
            if (count < MAX_PARAMS) args[count] = trim(text + start, i - start);
            count++;
            start = i + 1;
            if (c == ')') {
                *after = i + 1;
                return count;
            }
        }
        i++;
    }
    return -1;
}

static bool expand(const char *text, size_t length, const struct disabled *disabled,
                   struct buffer *out);

// Adds the macro's body to out, with each parameter replaced by its argument.
static bool substitute(const struct macro *macro, const struct slice *args,
                       const struct disabled *disabled, struct buffer *out) {
    // An argument is expanded only if it is used, and only once.
    struct buffer expanded[MAX_PARAMS] = {0};
    const char *body = macro->body;
    size_t length = strlen(body), i = 0;
    bool ok = true;
    while (ok && i < length) {
        size_t end = i + 1;
        if (body[i] == '"') {
            end = string_end(body, length, i);
        } else if (body[i] == '#') {
            // # and a parameter make a string of the argument as written.
            size_t start = i + 1;
            while (start < length && body[start] == ' ') start++;
            size_t stop = word_end(body, length, start);
            int p = find_param(macro, (struct slice){body + start, stop - start});
            if (p >= 0) {
                add(out, "\"", 1);
                add(out, args[p].text, args[p].length);
                add(out, "\"", 1);
                i = stop;
                continue;
            }
        } else if (is_word_char(body[i])) {
            end = word_end(body, length, i);
            int p = find_param(macro, (struct slice){body + i, end - i});
            if (p >= 0) {
                if (expanded[p].text == NULL) {
                    expanded[p] = new_buffer();
                    ok = expand(args[p].text, args[p].length, disabled, &expanded[p]);
                }
                add(out, expanded[p].text, expanded[p].length);
                i = end;
                continue;
            }
        }
        add(out, body + i, end - i);
        i = end;
    }
    for (int p = 0; p < MAX_PARAMS; p++) free(expanded[p].text);
    return ok;
}

// Expands a call to macro, whose "(" is at text[open], and sets *after to the
// index past its ")".
static bool expand_call(const struct macro *macro, const char *text, size_t length, size_t open,
                        const struct disabled *disabled, struct buffer *out, size_t *after) {
    struct slice args[MAX_PARAMS];
    int count = split_args(text, length, open, args, after);
    if (count < 0) {
        snprintf(error, sizeof error, "unterminated call to %s", macro->name);
        return false;
    }
    if (macro->param_count == 0 && count == 1 && args[0].length == 0) count = 0;
    if (count != macro->param_count) {
        snprintf(error, sizeof error, "%s takes %d argument%s, got %d", macro->name,
                 macro->param_count, macro->param_count == 1 ? "" : "s", count);
        return false;
    }
    struct buffer body = new_buffer();
    struct disabled inner = {macro, disabled};
    bool ok = substitute(macro, args, disabled, &body) &&
              expand(body.text, body.length, &inner, out);
    free(body.text);
    return ok;
}

// Adds text to out with every macro expanded, except those in disabled.
static bool expand(const char *text, size_t length, const struct disabled *disabled,
                   struct buffer *out) {
    size_t i = 0;
    while (i < length) {
        size_t end = i + 1;
        if (text[i] == '"') {
            end = string_end(text, length, i);
        } else if (is_word_char(text[i])) {
            end = word_end(text, length, i);
            const struct macro *macro = find_macro((struct slice){text + i, end - i});
            // Inside its own expansion, a macro's name is left alone.
            if (macro != NULL && !is_disabled(disabled, macro)) {
                if (macro->param_count < 0) {
                    struct disabled inner = {macro, disabled};
                    if (!expand(macro->body, strlen(macro->body), &inner, out)) return false;
                    i = end;
                    continue;
                }
                size_t paren = end;
                while (paren < length && text[paren] == ' ') paren++;
                if (paren < length && text[paren] == '(') {
                    if (!expand_call(macro, text, length, paren, disabled, out, &i)) return false;
                    continue;
                }
            }
        }
        add(out, text + i, end - i);
        i = end;
    }
    return true;
}

static void forget(struct macro *macro) {
    free(macro->name);
    free(macro->body);
    for (int i = 0; i < macro->param_count; i++) free(macro->params[i]);
}

// Defines the macro in rest, the text after "#define".
static bool define(struct slice rest) {
    size_t end = word_end(rest.text, rest.length, 0);
    struct slice name = {rest.text, end};
    if (!is_name(name)) return false;
    struct slice params[MAX_PARAMS];
    int count = -1;  // no parentheses
    struct slice body = trim(rest.text + end, rest.length - end);
    if (end < rest.length && rest.text[end] == '(') {
        const char *open = rest.text + end;
        const char *close = memchr(open, ')', rest.length - end);
        if (close == NULL) return false;
        struct slice inside = {open + 1, (size_t)(close - open - 1)};
        // Blank parentheses have no parameters. Otherwise, each comma ends one.
        bool blank = trim(inside.text, inside.length).length == 0;
        count = 0;
        for (size_t start = 0; !blank && start <= inside.length; count++) {
            const char *comma = memchr(inside.text + start, ',', inside.length - start);
            size_t stop = comma == NULL ? inside.length : (size_t)(comma - inside.text);
            if (count == MAX_PARAMS) return false;
            params[count] = trim(inside.text + start, stop - start);
            if (!is_name(params[count])) return false;
            for (int j = 0; j < count; j++) {
                if (equal(params[j], params[count])) return false;
            }
            start = stop + 1;
        }
        body = trim(close + 1, (size_t)(rest.text + rest.length - close - 1));
    }

    struct macro *macro = find_macro(name);
    if (macro != NULL) {
        forget(macro);
    } else if (macro_count < MAX_MACROS) {
        macro = &macros[macro_count++];
    } else {
        return false;
    }
    macro->name = copy(name);
    macro->param_count = count;
    for (int i = 0; i < count; i++) macro->params[i] = copy(params[i]);
    macro->body = copy(body);
    return true;
}

// Runs one line, a directive or text to expand and print. Returns false, with
// error set, for a line with a mistake in it.
static bool run(const char *line) {
    struct slice whole = trim(line, strlen(line));
    const char *space = memchr(whole.text, ' ', whole.length);
    struct slice directive = {whole.text, space ? (size_t)(space - whole.text) : whole.length};
    struct slice rest = trim(whole.text + directive.length, whole.length - directive.length);
    if (same(directive, "#define")) {
        if (define(rest)) return true;
        snprintf(error, sizeof error, "bad #define");
        return false;
    }
    if (same(directive, "#undef")) {
        if (!is_name(rest)) {
            snprintf(error, sizeof error, "bad #undef");
            return false;
        }
        struct macro *macro = find_macro(rest);
        if (macro != NULL) {
            forget(macro);
            *macro = macros[--macro_count];
        }
        return true;
    }
    struct buffer out = new_buffer();
    bool ok = expand(line, strlen(line), NULL, &out);
    if (ok) printf("%s\n", out.text);
    free(out.text);
    return ok;
}

int main(void) {
    char line[MAX_LINE + 2];
    while (fgets(line, sizeof line, stdin) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (!run(line)) printf("error: %s\n", error);
    }
    for (int i = 0; i < macro_count; i++) forget(&macros[i]);
    return 0;
}
