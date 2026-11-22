#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORKERS 16

// A hash table from words to counts. Its size is a power of 2, and it grows
// before it is half full, so a search always finds an empty slot.
struct entry {
    char *word;  // NULL in an empty slot
    long count;
};

struct table {
    struct entry *slots;
    size_t size, used;
};

static void *checked(void *memory) {
    if (memory == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return memory;
}

static size_t hash(const char *word, size_t length) {
    size_t h = 14695981039346656037u;  // FNV-1a
    for (size_t i = 0; i < length; i++) h = (h ^ (unsigned char)word[i]) * 1099511628211u;
    return h;
}

static struct entry *find(struct table *table, const char *word, size_t length) {
    size_t i = hash(word, length) & (table->size - 1);
    while (table->slots[i].word != NULL &&
           !(strlen(table->slots[i].word) == length &&
             memcmp(table->slots[i].word, word, length) == 0)) {
        i = (i + 1) & (table->size - 1);
    }
    return &table->slots[i];
}

static struct table new_table(size_t size) {
    return (struct table){checked(calloc(size, sizeof(struct entry))), size, 0};
}

static void grow(struct table *table) {
    struct table bigger = new_table(table->size * 2);
    for (size_t i = 0; i < table->size; i++) {
        struct entry old = table->slots[i];
        if (old.word == NULL) continue;
        *find(&bigger, old.word, strlen(old.word)) = old;
        bigger.used++;
    }
    free(table->slots);
    *table = bigger;
}

static void add(struct table *table, const char *word, size_t length, long count) {
    if (table->size == 0) *table = new_table(64);
    if (2 * (table->used + 1) > table->size) grow(table);
    struct entry *slot = find(table, word, length);
    if (slot->word == NULL) {
        slot->word = checked(malloc(length + 1));
        memcpy(slot->word, word, length);
        slot->word[length] = '\0';
        table->used++;
    }
    slot->count += count;
}

static void free_table(struct table *table) {
    for (size_t i = 0; i < table->size; i++) free(table->slots[i].word);
    free(table->slots);
}

// A worker's input and its result travel together in a struct. Each thread
// writes only to its own job, and has a table of its own.
struct job {
    char **lines;
    size_t count;
    struct table words;
    long total;
};

static bool is_letter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

static void *count_words(void *arg) {
    struct job *job = arg;
    char word[1000];
    for (size_t i = 0; i < job->count; i++) {
        const char *line = job->lines[i];
        for (size_t at = 0; line[at] != '\0';) {
            if (!is_letter(line[at])) {
                at++;
                continue;
            }
            size_t length = 0;
            while (is_letter(line[at])) {
                char c = line[at++];
                if (length < sizeof word) word[length++] = c >= 'A' && c <= 'Z' ? c + 'a' - 'A' : c;
            }
            add(&job->words, word, length, 1);
            job->total++;
        }
    }
    return NULL;
}

static const char *plural(long count) { return count == 1 ? "" : "s"; }

static int by_count_then_word(const void *a, const void *b) {
    const struct entry *x = a, *y = b;
    if (x->count != y->count) return x->count > y->count ? -1 : 1;
    return strcmp(x->word, y->word);
}

static char *read_all(FILE *file) {
    size_t length = 0, capacity = 4096;
    char *text = checked(malloc(capacity));
    size_t got;
    while ((got = fread(text + length, 1, capacity - length - 1, file)) > 0) {
        length += got;
        if (length + 1 == capacity) text = checked(realloc(text, capacity *= 2));
    }
    text[length] = '\0';
    return text;
}

// Splits text into lines in place, and returns where each one starts.
static char **split_lines(char *text, size_t *count) {
    size_t newlines = 0;
    for (char *at = text; *at != '\0'; at++) newlines += *at == '\n';
    char **lines = checked(malloc((newlines + 1) * sizeof(char *)));
    *count = 0;
    for (char *at = text; *at != '\0';) {
        lines[(*count)++] = at;
        char *newline = strchr(at, '\n');
        if (newline == NULL) break;
        *newline = '\0';
        at = newline + 1;
    }
    return lines;
}

static bool is_number(const char *word) {
    if (word == NULL || *word == '\0') return false;
    for (; *word != '\0'; word++) {
        if (*word < '0' || *word > '9') return false;
    }
    return true;
}

int main(void) {
    char *input = read_all(stdin), empty[] = "";
    size_t line_count;
    char **lines = split_lines(input, &line_count);
    char *first = line_count > 0 ? lines[0] : empty;
    char *workers_word = strtok(first, " \t"), *top_word = strtok(NULL, " \t");
    if (!is_number(workers_word) || !is_number(top_word) || strtok(NULL, " \t") != NULL) {
        printf("error: the first line must be two numbers, workers and top\n");
        free(lines);
        free(input);
        return 0;
    }
    long workers = strtol(workers_word, NULL, 10), top = strtol(top_word, NULL, 10);
    if (workers < 1 || workers > MAX_WORKERS) {
        printf("error: workers must be from 1 to %d\n", MAX_WORKERS);
        free(lines);
        free(input);
        return 0;
    }

    // Each worker gets text_count / workers lines, and the first few get one
    // more, so that every line is counted once.
    char **text = lines + 1;
    size_t text_count = line_count > 0 ? line_count - 1 : 0;
    pthread_t threads[MAX_WORKERS];
    struct job jobs[MAX_WORKERS];
    size_t start = 0;
    for (long i = 0; i < workers; i++) {
        size_t size = text_count / workers + ((size_t)i < text_count % workers ? 1 : 0);
        jobs[i] = (struct job){text + start, size, {NULL, 0, 0}, 0};
        start += size;
        if (pthread_create(&threads[i], NULL, count_words, &jobs[i]) != 0) {
            fprintf(stderr, "can't start thread %ld\n", i);
            return 1;
        }
    }

    // After a join, main can read what that worker wrote.
    printf("%ld worker%s, %zu line%s\n", workers, plural(workers), text_count,
           plural((long)text_count));
    struct table total = {NULL, 0, 0};
    long words = 0;
    for (long i = 0; i < workers; i++) {
        pthread_join(threads[i], NULL);
        printf("worker %ld: %zu line%s, %ld word%s\n", i, jobs[i].count,
               plural((long)jobs[i].count), jobs[i].total, plural(jobs[i].total));
        for (size_t j = 0; j < jobs[i].words.size; j++) {
            struct entry entry = jobs[i].words.slots[j];
            if (entry.word != NULL) add(&total, entry.word, strlen(entry.word), entry.count);
        }
        words += jobs[i].total;
        free_table(&jobs[i].words);
    }
    printf("%ld word%s, %zu different\n", words, plural(words), total.used);

    struct entry *sorted = checked(malloc((total.used + 1) * sizeof(struct entry)));
    size_t n = 0;
    for (size_t i = 0; i < total.size; i++) {
        if (total.slots[i].word != NULL) sorted[n++] = total.slots[i];
    }
    qsort(sorted, n, sizeof(struct entry), by_count_then_word);
    for (size_t i = 0; i < n && (long)i < top; i++) {
        printf("%s: %ld\n", sorted[i].word, sorted[i].count);
    }
    free(sorted);
    free_table(&total);
    free(lines);
    free(input);
    return 0;
}
