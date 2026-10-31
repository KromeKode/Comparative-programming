#include <stdio.h>

struct row {
    const char *name;
    int score;
};

// An interface in C is a struct of function pointers, often called a vtable.
// Every exporter starts with a pointer to one, and each kind of exporter
// fills in its own functions.
struct exporter;
struct exporter_vtable {
    const char *(*name)(const struct exporter *self);
    void (*export)(const struct exporter *self, const struct row *rows,
                   int count);
};

struct exporter {
    const struct exporter_vtable *vtable;
};

// A CSV exporter is an exporter plus its own data. Because base comes first,
// a pointer to a csv_exporter can be used as a pointer to an exporter.
struct csv_exporter {
    struct exporter base;
    char separator;
};

const char *csv_name(const struct exporter *self) {
    (void)self;
    return "csv";
}

void csv_export(const struct exporter *self, const struct row *rows,
                int count) {
    const struct csv_exporter *csv = (const struct csv_exporter *)self;
    printf("name%cscore\n", csv->separator);
    for (int i = 0; i < count; i++) {
        printf("%s%c%d\n", rows[i].name, csv->separator, rows[i].score);
    }
}

const char *json_name(const struct exporter *self) {
    (void)self;
    return "json";
}

void json_export(const struct exporter *self, const struct row *rows,
                 int count) {
    (void)self;
    printf("[");
    for (int i = 0; i < count; i++) {
        printf("%s{\"name\":\"%s\",\"score\":%d}", i > 0 ? "," : "",
               rows[i].name, rows[i].score);
    }
    printf("]\n");
}

const char *markdown_name(const struct exporter *self) {
    (void)self;
    return "markdown";
}

void markdown_export(const struct exporter *self, const struct row *rows,
                     int count) {
    (void)self;
    printf("| name | score |\n| --- | --- |\n");
    for (int i = 0; i < count; i++) {
        printf("| %s | %d |\n", rows[i].name, rows[i].score);
    }
}

const char *summary_name(const struct exporter *self) {
    (void)self;
    return "summary";
}

void summary_export(const struct exporter *self, const struct row *rows,
                    int count) {
    (void)self;
    int best = 0;
    for (int i = 1; i < count; i++) {
        if (rows[i].score > rows[best].score) {
            best = i;
        }
    }
    printf("%d rows, top score %d by %s\n", count, rows[best].score,
           rows[best].name);
}

// One vtable per kind, shared by every exporter of that kind.
static const struct exporter_vtable csv_vtable = {csv_name, csv_export};
static const struct exporter_vtable json_vtable = {json_name, json_export};
static const struct exporter_vtable markdown_vtable = {markdown_name,
                                                       markdown_export};
static const struct exporter_vtable summary_vtable = {summary_name,
                                                      summary_export};

// This function knows only the interface, not the kinds of exporter.
void run(const struct exporter *exporter, const struct row *rows, int count) {
    printf("== %s\n", exporter->vtable->name(exporter));
    exporter->vtable->export(exporter, rows, count);
}

int main(void) {
    struct row rows[] = {{"ada", 90}, {"grace", 85}, {"alan", 78}};
    struct csv_exporter csv = {{&csv_vtable}, ','};
    struct exporter json = {&json_vtable};
    struct exporter markdown = {&markdown_vtable};
    struct exporter summary = {&summary_vtable};
    const struct exporter *exporters[] = {&csv.base, &json, &markdown,
                                          &summary};
    for (int i = 0; i < 4; i++) {
        run(exporters[i], rows, 3);
    }
    return 0;
}
