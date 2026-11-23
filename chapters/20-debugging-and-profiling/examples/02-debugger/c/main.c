// Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
// input if there are none. Section 20.5 runs it in a debugger, to watch the parser
// call itself. It expects well-formed expressions, and doesn't check.
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// A recursive descent parser, with one function for each rule of the grammar.
struct parser {
    const char *text;
    size_t pos;
};

static long long expr(struct parser *p);

// Skips spaces, and gives the next character, or '\0' at the end.
static char peek(struct parser *p) {
    while (p->text[p->pos] == ' ') p->pos++;
    return p->text[p->pos];
}

// factor = number | "(" expr ")"
static long long factor(struct parser *p) {
    if (peek(p) == '(') {
        p->pos++;
        long long value = expr(p);
        peek(p);
        p->pos++;  // the ')'
        return value;
    }
    long long value = 0;
    while (isdigit((unsigned char)p->text[p->pos])) {
        value = value * 10 + (p->text[p->pos] - '0');
        p->pos++;
    }
    return value;
}

// term = factor { "*" factor }
static long long term(struct parser *p) {
    long long value = factor(p);
    while (peek(p) == '*') {
        p->pos++;
        value *= factor(p);
    }
    return value;
}

// expr = term { ("+" | "-") term }
static long long expr(struct parser *p) {
    long long value = term(p);
    while (peek(p) == '+' || peek(p) == '-') {
        char op = p->text[p->pos];
        p->pos++;
        long long right = term(p);
        value = op == '+' ? value + right : value - right;
    }
    return value;
}

static long long evaluate(const char *text) {
    struct parser p = {text, 0};
    return expr(&p);
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) printf("%lld\n", evaluate(argv[i]));
    if (argc > 1) return 0;
    char line[4096];
    while (fgets(line, sizeof line, stdin)) {
        line[strcspn(line, "\n")] = '\0';
        printf("%lld\n", evaluate(line));
    }
    return 0;
}
