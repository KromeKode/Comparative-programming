#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { CIRCLE, RECTANGLE, TRIANGLE } ShapeKind;

// A tagged union: kind says which member of the union is in use.
typedef struct {
    ShapeKind kind;
    union {
        struct {
            double radius;
        } circle;
        struct {
            double width, height;
        } rectangle;
        struct {
            double base, height;
        } triangle;
    };
} Shape;

static const double PI = 3.14159265358979323846;

static const struct {
    const char *name;
    ShapeKind kind;
    int sizes_needed;
} KINDS[] = {{"circle", CIRCLE, 1}, {"rectangle", RECTANGLE, 2}, {"triangle", TRIANGLE, 2}};

static double area(Shape shape) {
    switch (shape.kind) {
    case CIRCLE:
        return PI * shape.circle.radius * shape.circle.radius;
    case RECTANGLE:
        return shape.rectangle.width * shape.rectangle.height;
    case TRIANGLE:
        return shape.triangle.base * shape.triangle.height / 2;
    }
    return 0;
}

typedef enum { PARSED, BLANK, BAD } ParseResult;

// Fills *shape from one line. On BAD, error holds the message.
static ParseResult parse(char *line, Shape *shape, char *error, size_t size) {
    const char *separators = " \t\r\n";
    char *name = strtok(line, separators);
    if (name == NULL) {
        return BLANK;
    }
    char *texts[3];
    int count = 0;
    for (char *word = strtok(NULL, separators); word != NULL;
         word = strtok(NULL, separators)) {
        if (count < 3) {
            texts[count] = word;
        }
        count++;
    }

    int kind = -1;
    for (int i = 0; i < 3; i++) {
        if (strcmp(KINDS[i].name, name) == 0) {
            kind = i;
        }
    }
    if (kind < 0) {
        snprintf(error, size, "unknown shape \"%s\"", name);
        return BAD;
    }
    int needed = KINDS[kind].sizes_needed;
    if (count != needed) {
        snprintf(error, size, "%s needs %d size%s, got %d", name, needed,
                 needed == 1 ? "" : "s", count);
        return BAD;
    }

    double sizes[2] = {0, 0};
    for (int i = 0; i < needed; i++) {
        char *end;
        sizes[i] = strtod(texts[i], &end);
        if (end == texts[i] || *end != '\0') {
            snprintf(error, size, "\"%s\" is not a number", texts[i]);
            return BAD;
        }
        if (sizes[i] < 0) {
            snprintf(error, size, "\"%s\" is negative", texts[i]);
            return BAD;
        }
    }

    shape->kind = KINDS[kind].kind;
    switch (shape->kind) {
    case CIRCLE:
        shape->circle.radius = sizes[0];
        break;
    case RECTANGLE:
        shape->rectangle.width = sizes[0];
        shape->rectangle.height = sizes[1];
        break;
    case TRIANGLE:
        shape->triangle.base = sizes[0];
        shape->triangle.height = sizes[1];
        break;
    }
    return PARSED;
}

int main(void) {
    static char line[1 << 16];
    double total = 0;
    while (fgets(line, sizeof line, stdin) != NULL) {
        Shape shape;
        char error[256];
        switch (parse(line, &shape, error, sizeof error)) {
        case BLANK:
            break;
        case BAD:
            printf("error: %s\n", error);
            break;
        case PARSED:
            total += area(shape);
            printf("%s: %.2f\n", KINDS[shape.kind].name, area(shape));
            break;
        }
    }
    printf("total: %.2f\n", total);
    return 0;
}
