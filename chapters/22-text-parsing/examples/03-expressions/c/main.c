// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
#define _POSIX_C_SOURCE 200809L  // for getline, from POSIX
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C has no exceptions, so each function that can fail fills in an Error and says so.
typedef struct {
    int column;  // counted in characters, from 1
    char message[128];
} Error;

static void *checked(void *p) {
    if (p == NULL) {
        fputs("calc: out of memory\n", stderr);
        exit(1);
    }
    return p;
}

// The text is UTF-8, so a character is one to four bytes, and all but the first are
// 10xxxxxx. This is how many bytes the character that starts at text[i] has.
static size_t char_length(const char *text, size_t i) {
    size_t n = 1;
    while (((unsigned char)text[i + n] & 0xC0) == 0x80) n++;  // the '\0' ends it
    return n;
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
typedef enum { NUMBER_TOKEN, OPERATOR_TOKEN, END_TOKEN } Kind;  // operators include ( )

typedef struct {
    Kind kind;
    const char *text;  // in the line, not ended with a '\0'
    int length;
    int column;
    int64_t value;  // a number's value
} Token;

// Writes "the end of the line", or the token in quotes, into out.
static void describe(const Token *token, char *out, size_t size) {
    if (token->kind == END_TOKEN) {
        snprintf(out, size, "the end of the line");
    } else {
        snprintf(out, size, "\"%.*s\"", token->length, token->text);
    }
}

static bool is_digit(char c) { return '0' <= c && c <= '9'; }

// Returns the tokens, ended by an END_TOKEN, or NULL with error filled in.
static Token *tokens(const char *line, Error *error) {
    size_t count = 0, capacity = 16;
    Token *found = checked(malloc(capacity * sizeof *found));
    int column = 1;
    size_t i = 0;
    for (; line[i] != '\0'; column++) {
        if (count + 1 == capacity) {  // room for this token and the END_TOKEN
            capacity *= 2;
            found = checked(realloc(found, capacity * sizeof *found));
        }
        char c = line[i];
        if (c == ' ' || c == '\t') {
            i++;
        } else if (is_digit(c)) {
            size_t start = i;
            while (is_digit(line[i])) i++;
            // strtoll stops at the first character that isn't a digit. It would also
            // take spaces and a sign before them, but there are none.
            errno = 0;
            int64_t value = strtoll(line + start, NULL, 10);
            if (errno == ERANGE) {
                *error = (Error){.column = column, .message = "number too large for 64 bits"};
                free(found);
                return NULL;
            }
            int length = (int)(i - start);
            found[count++] = (Token){NUMBER_TOKEN, line + start, length, column, value};
            column += length - 1;
        } else if (strchr("+-*/%^()", c) != NULL) {
            found[count++] = (Token){OPERATOR_TOKEN, line + i, 1, column, 0};
            i++;
        } else {
            error->column = column;
            snprintf(error->message, sizeof error->message, "unexpected \"%.*s\"",
                     (int)char_length(line, i), line + i);
            free(found);
            return NULL;
        }
    }
    found[count] = (Token){END_TOKEN, line + i, 0, column, 0};
    return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. C has no sum
// types, so a node is a struct with a kind, and a union of what each kind holds. Only
// the member for the kind may be used. column is where the operator is, for an error
// such as overflow.
typedef struct Expr Expr;
struct Expr {
    enum { NUMBER, NEGATE, BINARY } kind;
    int column;
    union {
        int64_t value;                                   // NUMBER
        Expr *operand;                                   // NEGATE
        struct { char op; Expr *left, *right; } binary;  // BINARY
    };
};

static void free_expr(Expr *expr) {
    if (expr == NULL) return;
    if (expr->kind == NEGATE) free_expr(expr->operand);
    if (expr->kind == BINARY) {
        free_expr(expr->binary.left);
        free_expr(expr->binary.right);
    }
    free(expr);
}

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2). Returns false if token
// isn't an infix operator.
static bool infix(const Token *token, int *left, int *right) {
    if (token->kind != OPERATOR_TOKEN) return false;
    switch (token->text[0]) {
        case '+': case '-': *left = 1; *right = 2; return true;
        case '*': case '/': case '%': *left = 3; *right = 4; return true;
        case '^': *left = 8; *right = 7; return true;
        default: return false;
    }
}

// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
enum { PREFIX = 5 };

typedef struct {
    const Token *tokens;
    size_t at;
    Error *error;
} Parser;

static const Token *next(Parser *p) {
    const Token *token = &p->tokens[p->at];
    if (token->kind != END_TOKEN) p->at++;
    return token;
}

static void fail(Parser *p, const Token *token, const char *expected) {
    char found[64];
    describe(token, found, sizeof found);
    p->error->column = token->column;
    snprintf(p->error->message, sizeof p->error->message, "expected %s, found %s", expected,
             found);
}

static Expr *new_expr(Expr node) {
    Expr *expr = checked(malloc(sizeof *expr));
    *expr = node;
    return expr;
}

// Parses the longest expression whose operators hold at least min_power. Returns NULL
// with the error filled in if it can't.
static Expr *expression(Parser *p, int min_power) {
    const Token *token = next(p);
    Expr *left;
    if (token->kind == NUMBER_TOKEN) {
        left = new_expr((Expr){.kind = NUMBER, .value = token->value});
    } else if (token->text[0] == '-') {
        Expr *operand = expression(p, PREFIX);
        if (operand == NULL) return NULL;
        left = new_expr((Expr){.kind = NEGATE, .column = token->column, .operand = operand});
    } else if (token->text[0] == '(') {
        left = expression(p, 0);
        if (left == NULL) return NULL;
        const Token *close = next(p);
        if (close->kind != OPERATOR_TOKEN || close->text[0] != ')') {
            fail(p, close, "\")\"");
            free_expr(left);
            return NULL;
        }
    } else {
        fail(p, token, "a number, \"(\", or \"-\"");
        return NULL;
    }
    int left_power, right_power;
    while (infix(&p->tokens[p->at], &left_power, &right_power) && left_power >= min_power) {
        const Token *op = next(p);
        Expr *right = expression(p, right_power);
        if (right == NULL) {
            free_expr(left);
            return NULL;
        }
        left = new_expr((Expr){.kind = BINARY, .column = op->column,
                               .binary = {op->text[0], left, right}});
    }
    return left;
}

// Returns the tree for line, or NULL with error filled in.
static Expr *parse(const char *line, Error *error) {
    Token *found = tokens(line, error);
    if (found == NULL) return NULL;
    Parser p = {found, 0, error};
    Expr *expr = expression(&p, 0);
    if (expr != NULL) {
        const Token *token = next(&p);
        if (token->kind != END_TOKEN) {
            fail(&p, token, "an operator");
            free_expr(expr);
            expr = NULL;
        }
    }
    free(found);
    return expr;
}

static void show(const Expr *expr) {
    switch (expr->kind) {
        case NUMBER: printf("%" PRId64, expr->value); break;
        case NEGATE:
            printf("(-");
            show(expr->operand);
            printf(")");
            break;
        case BINARY:
            printf("(");
            show(expr->binary.left);
            printf(" %c ", expr->binary.op);
            show(expr->binary.right);
            printf(")");
            break;
    }
}

static bool error_at(Error *error, int column, const char *message) {
    error->column = column;
    snprintf(error->message, sizeof error->message, "%s", message);
    return false;
}

// Signed overflow is undefined behavior in C, so the answer must be checked before it's
// made. GCC and Clang's __builtin_mul_overflow and the others work out the answer, and
// say whether it overflowed; C23 has them as ckd_mul and the others, in <stdckdint.h>.
static bool power(int64_t base, int64_t exponent, int64_t *result, int column, Error *error) {
    *result = 1;
    // Squares the base for each bit of the exponent, and stops at the first overflow.
    while (exponent > 0) {
        if (exponent % 2 == 1 && __builtin_mul_overflow(*result, base, result)) {
            return error_at(error, column, "overflow");
        }
        exponent /= 2;
        if (exponent > 0 && __builtin_mul_overflow(base, base, &base)) {
            return error_at(error, column, "overflow");
        }
    }
    return true;
}

// Works out the value of expr into *value. Returns false with error filled in if it
// can't.
static bool evaluate(const Expr *expr, int64_t *value, Error *error) {
    switch (expr->kind) {
        case NUMBER:
            *value = expr->value;
            return true;
        case NEGATE:
            if (!evaluate(expr->operand, value, error)) return false;
            if (*value == INT64_MIN) return error_at(error, expr->column, "overflow");
            *value = -*value;
            return true;
        case BINARY: break;
    }
    int64_t a, b;
    if (!evaluate(expr->binary.left, &a, error) || !evaluate(expr->binary.right, &b, error)) {
        return false;
    }
    char op = expr->binary.op;
    int column = expr->column;
    if ((op == '/' || op == '%') && b == 0) return error_at(error, column, "division by zero");
    bool overflow = false;
    switch (op) {
        case '+': overflow = __builtin_add_overflow(a, b, value); break;
        case '-': overflow = __builtin_sub_overflow(a, b, value); break;
        case '*': overflow = __builtin_mul_overflow(a, b, value); break;
        // / and % round toward zero. INT64_MIN / -1 and INT64_MIN % -1 are undefined,
        // since the quotient doesn't fit; the remainder is 0.
        case '/':
            overflow = a == INT64_MIN && b == -1;
            if (!overflow) *value = a / b;
            break;
        case '%': *value = b == -1 ? 0 : a % b; break;
        default:  // '^'
            if (b < 0) return error_at(error, column, "negative exponent");
            return power(a, b, value, column, error);
    }
    return overflow ? error_at(error, column, "overflow") : true;
}

int main(void) {
    int status = 0;
    char *line = NULL;
    size_t size = 0;
    ssize_t length;
    // getline keeps the '\n', and a '\r' before it.
    for (long number = 1; (length = getline(&line, &size, stdin)) != -1; number++) {
        if (length > 0 && line[length - 1] == '\n') line[--length] = '\0';
        if (length > 0 && line[length - 1] == '\r') line[--length] = '\0';
        if (line[strspn(line, " \t")] == '\0') continue;
        Error error;
        int64_t value;
        Expr *expr = parse(line, &error);
        if (expr != NULL && evaluate(expr, &value, &error)) {
            show(expr);
            printf(" = %" PRId64 "\n", value);
        } else {
            fprintf(stderr, "calc: line %ld, column %d: %s\n  %s\n  ", number, error.column,
                    error.message, line);
            // The caret goes under the column. A tab before it stays a tab, so that the
            // caret lines up with the text whatever a tab's width is.
            for (size_t i = 0, column = 1; column < (size_t)error.column; column++) {
                fputc(line[i] == '\t' ? '\t' : ' ', stderr);
                i += char_length(line, i);
            }
            fputs("^\n", stderr);
            status = 1;
        }
        free_expr(expr);
    }
    free(line);
    return status;
}
