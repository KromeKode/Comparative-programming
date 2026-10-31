#include <stdbool.h>
#include <stdio.h>

// A pretend file that prints when it opens and closes.
typedef struct {
    const char *name;
} File;

bool file_open(File *f, const char *name, bool fails) {
    if (fails) {
        return false;
    }
    f->name = name;
    printf("  open %s\n", name);
    return true;
}

void file_close(File *f) {
    printf("  close %s\n", f->name);
}

// Returns NULL on success, or a message saying what failed. Every exit after
// the first open goes through the labels at the bottom, which close what was
// opened in the reverse order.
const char *run_job(bool output_fails, bool copy_fails) {
    const char *error = NULL;
    File input;
    File output;
    if (!file_open(&input, "input", false)) {
        return "cannot open input";
    }
    if (!file_open(&output, "output", output_fails)) {
        error = "cannot open output";
        goto close_input;
    }
    if (copy_fails) {
        error = "disk full";
        goto close_output;
    }
    printf("  copy input to output\n");

close_output:
    file_close(&output);
close_input:
    file_close(&input);
    return error;
}

int main(void) {
    struct {
        const char *title;
        bool output_fails;
        bool copy_fails;
    } jobs[] = {
        {"everything works", false, false},
        {"opening output fails", true, false},
        {"copying fails", false, true},
    };
    for (int i = 0; i < 3; i++) {
        printf("job %d: %s\n", i + 1, jobs[i].title);
        const char *error = run_job(jobs[i].output_fails, jobs[i].copy_fails);
        if (error == NULL) {
            printf("  job %d done\n", i + 1);
        } else {
            printf("  job %d failed: %s\n", i + 1, error);
        }
    }
    return 0;
}
