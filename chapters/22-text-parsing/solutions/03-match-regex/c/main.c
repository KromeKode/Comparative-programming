// Prints each line of stdin that has a match for the pattern, as grep does, with a
// matcher of its own: the pattern becomes a tree, the tree becomes a program for a
// Thompson NFA, and the NFA reads each line once, keeping the set of states it could be
// in. So a line takes time in proportion to its length times the pattern's, whatever
// the pattern is.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t Rune;  // a code point
#define END ((Rune)0xFFFFFFFF)  // stands for the end of the pattern

static void *allocate(size_t size) {
    void *memory = malloc(size ? size : 1);
    if (!memory) {
        perror("rematch");
        exit(2);
    }
    return memory;
}

// Makes room for one more element in array, which has count of them and room for *cap.
static void *grow(void *array, size_t *cap, size_t count, size_t size) {
    if (count < *cap) return array;
    *cap = *cap ? 2 * *cap : 16;
    void *bigger = realloc(array, *cap * size);
    if (!bigger) {
        perror("rematch");
        exit(2);
    }
    return bigger;
}

// Decodes n bytes of UTF-8, which the input and the pattern are, into out, which has
// room for n code points, and returns how many there are.
static size_t decode(const char *s, size_t n, Rune *out) {
    size_t count = 0;
    for (size_t i = 0; i < n;) {
        unsigned char c = (unsigned char)s[i];
        size_t length = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
        Rune code = length == 1 ? c : c & (0x7F >> length);  // the bits after the 1s and the 0
        for (size_t k = 1; k < length && i + k < n; k++) {
            code = code << 6 | ((unsigned char)s[i + k] & 0x3F);
        }
        out[count++] = code;
        i += length;
    }
    return count;
}

typedef struct {
    Rune low, high;
} Range;

// The tree, in one array, with nodes that refer to each other by index. A character, a
// ".", and brackets are each a set of characters: "a" is the range a to a, and "." is
// the empty set, negated.
typedef enum { CHARS, ANCHOR, CONCAT, ALTERNATION, REPEAT } Kind;

typedef struct {
    Kind kind;
    size_t ranges, nranges;  // a set's: where its ranges start in the parser's, and how many
    bool negated;            // a set's
    Rune op;                 // '^' or '$' for an anchor; '*', '+', or '?' for a repeat
    int first, next;  // the first child, and the next of its parent's children; -1 for none
} Node;

// Reads alternation = concat ("|" concat)*, concat = repeat*, and repeat = atom [*+?].
// Each function returns a node's index, or -1 for an error, which it puts in error and
// error_at.
typedef struct {
    const Rune *pattern;  // code points, so that an index counts characters, as columns do
    size_t length, i;
    Node *nodes;
    size_t nnodes, nodes_cap;
    Range *ranges;
    size_t nranges, ranges_cap;
    const char *error;
    size_t error_at;
} Parser;

static int fail(Parser *p, size_t at, const char *message) {
    p->error = message;
    p->error_at = at;
    return -1;
}

static Rune peek(const Parser *p) { return p->i < p->length ? p->pattern[p->i] : END; }

static int new_node(Parser *p, Kind kind) {
    p->nodes = grow(p->nodes, &p->nodes_cap, p->nnodes, sizeof *p->nodes);
    p->nodes[p->nnodes] = (Node){.kind = kind, .ranges = p->nranges, .first = -1, .next = -1};
    return (int)p->nnodes++;
}

// Makes child the one after last among node's children, and returns it.
static int add_child(Parser *p, int node, int last, int child) {
    if (last < 0) {
        p->nodes[node].first = child;
    } else {
        p->nodes[last].next = child;
    }
    return child;
}

// Adds a range to node, the newest node, whose ranges are the newest.
static void add_range(Parser *p, int node, Rune low, Rune high) {
    p->ranges = grow(p->ranges, &p->ranges_cap, p->nranges, sizeof *p->ranges);
    p->ranges[p->nranges++] = (Range){low, high};
    p->nodes[node].nranges++;
}

// Whether "\" can make c a literal: the characters that mean something, inside brackets
// or out.
static bool is_special(Rune c) {
    static const char special[] = "\\.[]()*+?|^$-";
    return c < 0x80 && memchr(special, (int)c, sizeof special - 1) != NULL;
}

static bool is_repeat(Rune c) { return c == '*' || c == '+' || c == '?'; }

// Puts in *c a character, or a "\" and the character it makes a literal.
static bool character(Parser *p, Rune *c) {
    *c = p->pattern[p->i];
    if (*c != '\\') {
        p->i++;
        return true;
    }
    if (p->i + 1 == p->length) {
        fail(p, p->i, "trailing backslash");
        return false;
    }
    *c = p->pattern[p->i + 1];
    if (!is_special(*c)) {
        fail(p, p->i, "invalid escape");
        return false;
    }
    p->i += 2;
    return true;
}

static int brackets(Parser *p) {
    int node = new_node(p, CHARS);
    size_t start = p->i++;  // the [
    if (peek(p) == '^') {
        p->nodes[node].negated = true;
        p->i++;
    }
    if (peek(p) == ']') return fail(p, start, "empty brackets");
    while (peek(p) != ']') {
        if (peek(p) == END) return fail(p, start, "unclosed \"[\"");
        size_t at = p->i;
        Rune low, high;
        if (!character(p, &low)) return -1;
        high = low;
        // A "-" makes a range, unless it's last: "[a-]" is "a" and "-".
        if (peek(p) == '-' && p->i + 1 < p->length && p->pattern[p->i + 1] != ']') {
            p->i++;
            if (!character(p, &high)) return -1;
            if (high < low) return fail(p, at, "range out of order");
        }
        add_range(p, node, low, high);
    }
    p->i++;  // the ]
    return node;
}

static int alternation(Parser *p);

static int atom(Parser *p) {
    if (peek(p) == '(') {
        size_t start = p->i++;
        int node = alternation(p);
        if (node < 0) return -1;
        if (peek(p) != ')') return fail(p, start, "unclosed \"(\"");
        p->i++;
        return node;
    }
    if (peek(p) == '[') return brackets(p);
    int node = new_node(p, CHARS);
    if (peek(p) == '.') {
        p->i++;
        p->nodes[node].negated = true;
        return node;
    }
    Rune c;
    if (!character(p, &c)) return -1;
    add_range(p, node, c, c);
    return node;
}

static int repeat(Parser *p) {
    Rune c = peek(p);
    // At the start, or after "(", "|", "^", "$", or another "*", "+", or "?".
    if (is_repeat(c)) return fail(p, p->i, "nothing to repeat");
    if (c == '^' || c == '$') {
        p->i++;
        int node = new_node(p, ANCHOR);
        p->nodes[node].op = c;
        return node;
    }
    int item = atom(p);
    if (item < 0 || !is_repeat(peek(p))) return item;
    int node = new_node(p, REPEAT);
    p->nodes[node].op = p->pattern[p->i++];
    p->nodes[node].first = item;
    return node;
}

static int concat(Parser *p) {
    int node = new_node(p, CONCAT), last = -1;
    for (Rune c = peek(p); c != END && c != '|' && c != ')'; c = peek(p)) {
        int item = repeat(p);
        if (item < 0) return -1;
        last = add_child(p, node, last, item);
    }
    return node;
}

static int alternation(Parser *p) {
    int first = concat(p);
    if (first < 0 || peek(p) != '|') return first;
    int node = new_node(p, ALTERNATION);
    int last = add_child(p, node, -1, first);
    while (peek(p) == '|') {
        p->i++;
        int option = concat(p);
        if (option < 0) return -1;
        last = add_child(p, node, last, option);
    }
    return node;
}

static int parse(Parser *p) {
    int node = alternation(p);
    // alternation() stops early only at a ")".
    if (node >= 0 && p->i < p->length) return fail(p, p->i, "unmatched \")\"");
    return node;
}

// The program. OP_CHARS reads a character, if it's in the set, and goes on to the next
// instruction; OP_SPLIT goes on to both x and y; OP_JUMP goes on to x; OP_LINE_START and
// OP_LINE_END go on if the position is the start or the end of the line; and OP_MATCH is
// a match.
typedef enum { OP_CHARS, OP_SPLIT, OP_JUMP, OP_LINE_START, OP_LINE_END, OP_MATCH } Op;

typedef struct {
    Op op;
    const Range *ranges;  // OP_CHARS's, in the parser's
    size_t nranges;
    bool negated;
    size_t x, y;
} Instruction;

typedef struct {
    Instruction *code;
    size_t count, cap;
} Program;

static size_t emit(Program *program, Instruction instruction) {
    program->code = grow(program->code, &program->cap, program->count, sizeof *program->code);
    program->code[program->count] = instruction;
    return program->count++;
}

static void compile(const Parser *p, int n, Program *program);

// An alternation's options from option on:
// split L1, L2; L1: option; jump END; L2: the rest; END:
static void compile_options(const Parser *p, int option, Program *program) {
    if (p->nodes[option].next < 0) {
        compile(p, option, program);
        return;
    }
    size_t split = emit(program, (Instruction){.op = OP_SPLIT, .x = program->count + 1});
    compile(p, option, program);
    size_t jump = emit(program, (Instruction){.op = OP_JUMP});
    program->code[split].y = program->count;
    compile_options(p, p->nodes[option].next, program);
    program->code[jump].x = program->count;
}

// Appends node n's instructions to the program. They refer to each other by index,
// since grow can move them.
static void compile(const Parser *p, int n, Program *program) {
    const Node *node = &p->nodes[n];
    size_t start = program->count;
    switch (node->kind) {
        case CHARS:
            emit(program, (Instruction){.op = OP_CHARS, .ranges = p->ranges + node->ranges,
                                        .nranges = node->nranges, .negated = node->negated});
            break;
        case ANCHOR:
            emit(program, (Instruction){.op = node->op == '^' ? OP_LINE_START : OP_LINE_END});
            break;
        case CONCAT:
            for (int item = node->first; item >= 0; item = p->nodes[item].next) {
                compile(p, item, program);
            }
            break;
        case ALTERNATION:
            compile_options(p, node->first, program);
            break;
        case REPEAT:
            if (node->op == '*') {
                // L1: split L2, L3; L2: item; jump L1; L3:
                emit(program, (Instruction){.op = OP_SPLIT, .x = start + 1});
                compile(p, node->first, program);
                emit(program, (Instruction){.op = OP_JUMP, .x = start});
                program->code[start].y = program->count;
            } else if (node->op == '+') {
                // L1: item; split L1, L2; L2:
                compile(p, node->first, program);
                emit(program, (Instruction){.op = OP_SPLIT, .x = start, .y = program->count + 1});
            } else {
                // split L1, L2; L1: item; L2:
                emit(program, (Instruction){.op = OP_SPLIT, .x = start + 1});
                compile(p, node->first, program);
                program->code[start].y = program->count;
            }
            break;
    }
}

static bool accepts(const Instruction *in, Rune c) {
    bool inside = false;
    for (size_t k = 0; k < in->nranges && !inside; k++) {
        inside = in->ranges[k].low <= c && c <= in->ranges[k].high;
    }
    return inside != in->negated;
}

// The program, and room to run it: stack has room for 3 times the program's size and 1
// more, and reached and seen for its size.
typedef struct {
    Program program;
    size_t *stack, *reached;
    bool *seen;
} Matcher;

// Follows the n instructions on the stack without reading a character, at the position
// at, and puts the OP_CHARS instructions they lead to in reached, and how many in *count.
// Returns whether one of them leads to OP_MATCH.
static bool follow(Matcher *m, size_t n, size_t at, size_t length, size_t *count) {
    const Instruction *code = m->program.code;
    memset(m->seen, 0, m->program.count * sizeof *m->seen);
    *count = 0;
    while (n > 0) {
        size_t pc = m->stack[--n];
        if (m->seen[pc]) continue;  // so that a loop such as (a*)* doesn't go around forever
        m->seen[pc] = true;
        switch (code[pc].op) {
            case OP_CHARS:
                m->reached[(*count)++] = pc;
                break;
            case OP_SPLIT:
                m->stack[n++] = code[pc].y;
                m->stack[n++] = code[pc].x;
                break;
            case OP_JUMP:
                m->stack[n++] = code[pc].x;
                break;
            case OP_LINE_START:
                if (at == 0) m->stack[n++] = pc + 1;
                break;
            case OP_LINE_END:
                if (at == length) m->stack[n++] = pc + 1;
                break;
            case OP_MATCH:
                return true;
        }
    }
    return false;
}

// Whether a match starts anywhere in the line: instruction 0 starts at each position.
static bool search(Matcher *m, const Rune *line, size_t length) {
    size_t n = 0;  // how many instructions wait on the stack for the character at at
    for (size_t at = 0; at <= length; at++) {
        m->stack[n] = 0;
        size_t count;
        if (follow(m, n + 1, at, length, &count)) return true;
        n = 0;
        for (size_t k = 0; at < length && k < count; k++) {
            size_t pc = m->reached[k];
            if (accepts(&m->program.code[pc], line[at])) m->stack[n++] = pc + 1;
        }
    }
    return false;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("usage: rematch PATTERN\n", stderr);
        return 2;
    }
    size_t bytes = strlen(argv[1]);
    Rune *pattern = allocate(bytes * sizeof *pattern);
    Parser p = {.pattern = pattern, .length = decode(argv[1], bytes, pattern)};
    int tree = parse(&p);
    if (tree < 0) {
        fprintf(stderr, "rematch: column %zu: %s\n", p.error_at + 1, p.error);
        free(p.nodes);
        free(p.ranges);
        free(pattern);
        return 2;
    }
    Matcher m = {0};
    compile(&p, tree, &m.program);
    emit(&m.program, (Instruction){.op = OP_MATCH});
    size_t size = m.program.count;
    m.stack = allocate((3 * size + 1) * sizeof *m.stack);
    m.reached = allocate(size * sizeof *m.reached);
    m.seen = allocate(size * sizeof *m.seen);

    char *text = NULL;
    size_t length = 0, cap = 0, got;
    do {
        text = grow(text, &cap, length, 1);
        got = fread(text + length, 1, cap - length, stdin);
        length += got;
    } while (got > 0);
    Rune *line = allocate(length * sizeof *line);
    bool found = false;
    // Each line ends at a "\n", or at the end of the text if it doesn't end with one.
    for (size_t start = 0; start < length;) {
        const char *newline = memchr(text + start, '\n', length - start);
        size_t stop = newline ? (size_t)(newline - text) : length;
        if (search(&m, line, decode(text + start, stop - start, line))) {
            fwrite(text + start, 1, stop - start, stdout);
            putchar('\n');
            found = true;
        }
        start = stop + 1;
    }
    free(line);
    free(text);
    free(m.seen);
    free(m.reached);
    free(m.stack);
    free(m.program.code);
    free(p.nodes);
    free(p.ranges);
    free(pattern);
    return found ? 0 : 1;
}
