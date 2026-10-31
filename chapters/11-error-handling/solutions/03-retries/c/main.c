#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_ATTEMPTS 3
#define MAX_JOBS 100
#define MAX_ANSWERS 10

enum error {
    NO_ERROR,
    TIMED_OUT,
    SERVER_BUSY,
    NOT_FOUND,
    LOGIN_REFUSED,
    GAVE_UP,
};

const char *message(enum error error) {
    switch (error) {
    case NO_ERROR:
        return "no error";
    case TIMED_OUT:
        return "timed out";
    case SERVER_BUSY:
        return "server busy";
    case NOT_FOUND:
        return "not found";
    case LOGIN_REFUSED:
        return "login refused";
    case GAVE_UP:
        return "gave up";
    }
    return "?";
}

// Try the same file again?
bool is_retryable(enum error error) {
    return error == TIMED_OUT || error == SERVER_BUSY;
}

// Stop the whole batch?
bool is_fatal(enum error error) { return error == LOGIN_REFUSED; }

struct job {
    char name[64];
    enum error answers[MAX_ANSWERS]; // NO_ERROR means the download works
    int answer_count;
};

// The pretend server: the answer for this attempt. The last answer repeats.
enum error download(const struct job *job, int attempt) {
    int index = attempt < job->answer_count ? attempt : job->answer_count;
    return job->answers[index - 1];
}

// What download_with_retries did: how many attempts, and how it ended.
struct outcome {
    enum error error;
    int attempts;
    enum error last; // for GAVE_UP: the error from the last attempt
};

struct outcome download_with_retries(const struct job *job) {
    int wait = 1;
    for (int attempt = 1;; attempt++) {
        enum error error = download(job, attempt);
        if (error == NO_ERROR || !is_retryable(error)) {
            return (struct outcome){error, attempt, error};
        }
        if (attempt == MAX_ATTEMPTS) {
            return (struct outcome){GAVE_UP, attempt, error};
        }
        printf("%s: %s, retrying in %ds\n", job->name, message(error), wait);
        wait *= 2;
    }
}

enum error parse_answer(const char *word) {
    if (strcmp(word, "timeout") == 0) {
        return TIMED_OUT;
    }
    if (strcmp(word, "busy") == 0) {
        return SERVER_BUSY;
    }
    if (strcmp(word, "missing") == 0) {
        return NOT_FOUND;
    }
    if (strcmp(word, "auth") == 0) {
        return LOGIN_REFUSED;
    }
    return NO_ERROR;
}

static struct job jobs[MAX_JOBS];

int main(void) {
    int job_count = 0;
    char line[512];
    while (job_count < MAX_JOBS && fgets(line, sizeof line, stdin) != NULL) {
        struct job *job = &jobs[job_count];
        char *word = strtok(line, " \n");
        if (word == NULL) {
            continue;
        }
        snprintf(job->name, sizeof job->name, "%s", word);
        job->answer_count = 0;
        while ((word = strtok(NULL, " \n")) != NULL &&
               job->answer_count < MAX_ANSWERS) {
            job->answers[job->answer_count++] = parse_answer(word);
        }
        job_count++;
    }

    int downloaded = 0, skipped = 0, failed = 0, not_tried = 0;
    for (int i = 0; i < job_count; i++) {
        const char *name = jobs[i].name;
        struct outcome outcome = download_with_retries(&jobs[i]);
        if (outcome.error == NO_ERROR) {
            printf("%s: ok after %d attempt%s\n", name, outcome.attempts,
                   outcome.attempts == 1 ? "" : "s");
            downloaded++;
        } else if (outcome.error == GAVE_UP) {
            printf("%s: failed: gave up after %d attempts: %s\n", name,
                   outcome.attempts, message(outcome.last));
            failed++;
        } else if (is_fatal(outcome.error)) {
            printf("%s: stopped: %s\n", name, message(outcome.error));
            failed++;
            not_tried = job_count - i - 1;
            break;
        } else {
            printf("%s: skipped: %s\n", name, message(outcome.error));
            skipped++;
        }
    }
    printf("downloaded %d, skipped %d, failed %d, not tried %d\n", downloaded,
           skipped, failed, not_tried);
    return 0;
}
