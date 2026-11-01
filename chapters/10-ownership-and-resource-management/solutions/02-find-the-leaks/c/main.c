#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Every allocation goes through these two functions, which count the blocks
// that are still allocated. The program prints the count when it ends.
static size_t live_blocks = 0;

void *counted_malloc(size_t size) {
    void *block = malloc(size);
    if (block == NULL) {
        fputs("out of memory\n", stderr);
        exit(1);
    }
    live_blocks++;
    return block;
}

void counted_free(void *block) {
    if (block != NULL) {
        live_blocks--;
        free(block);
    }
}

char *copy_string(const char *text) {
    size_t size = strlen(text) + 1;
    char *copy = counted_malloc(size);
    memcpy(copy, text, size);
    return copy;
}

typedef struct Song {
    char *title;       // owned
    char *artist;      // owned
    struct Song *next; // not owned: the playlist owns every song
} Song;

typedef struct {
    Song *first;
} Playlist;

Song *song_new(const char *title, const char *artist) {
    Song *song = counted_malloc(sizeof *song);
    song->title = copy_string(title);
    song->artist = copy_string(artist);
    song->next = NULL;
    return song;
}

// Frees a song and the two strings it owns: three blocks.
void song_free(Song *song) {
    counted_free(song->title);
    counted_free(song->artist);
    counted_free(song);
}

// Returns the link that points at the song called title, or the link at the
// end of the list if there is no such song.
Song **find(Playlist *list, const char *title) {
    Song **link = &list->first;
    while (*link != NULL && strcmp((*link)->title, title) != 0) {
        link = &(*link)->next;
    }
    return link;
}

// Returns a new string that the caller must free.
char *describe(const Song *song) {
    size_t size = strlen(song->title) + strlen(song->artist) + 5;
    char *text = counted_malloc(size);
    snprintf(text, size, "%s by %s", song->title, song->artist);
    return text;
}

void add(Playlist *list, const char *title, const char *artist) {
    Song *song = song_new(title, artist);
    Song **link = find(list, title);
    if (*link != NULL) {
        printf("error: \"%s\" is already in the playlist\n", title);
        song_free(song); // fix 1: the new song was never added
        return;
    }
    *link = song;
    printf("added %s\n", title);
}

void remove_song(Playlist *list, const char *title) {
    Song **link = find(list, title);
    Song *song = *link;
    if (song == NULL) {
        printf("error: no song \"%s\"\n", title);
        return;
    }
    *link = song->next;
    song_free(song); // fix 2: free the strings too, not only the song
    printf("removed %s\n", title);
}

void rename_song(Playlist *list, const char *old_title, const char *new_title) {
    Song *song = *find(list, old_title);
    if (song == NULL) {
        printf("error: no song \"%s\"\n", old_title);
        return;
    }
    if (*find(list, new_title) != NULL) {
        printf("error: \"%s\" is already in the playlist\n", new_title);
        return;
    }
    counted_free(song->title); // fix 3: free the old title before replacing it
    song->title = copy_string(new_title);
    printf("renamed %s to %s\n", old_title, new_title);
}

void show(Playlist *list) {
    if (list->first == NULL) {
        printf("(empty)\n");
    }
    int number = 0;
    for (Song *song = list->first; song != NULL; song = song->next) {
        char *text = describe(song);
        printf("%d. %s\n", ++number, text);
        counted_free(text); // fix 4: describe's caller owns the string
    }
}

// Empties the playlist and returns how many songs it had.
int clear(Playlist *list) {
    int count = 0;
    // fix 5: with a garbage collector, setting first to NULL would be enough.
    // In C it frees nothing. Free each song, saving its next pointer first.
    Song *song = list->first;
    while (song != NULL) {
        Song *next = song->next;
        song_free(song);
        song = next;
        count++;
    }
    list->first = NULL;
    return count;
}

int takes(const char *command, int want, int got) {
    if (got != want) {
        printf("error: %s takes %d word%s, got %d\n", command, want,
               want == 1 ? "" : "s", got);
    }
    return got == want;
}

int main(void) {
    Playlist list = {NULL};
    char line[1000];
    while (fgets(line, sizeof line, stdin) != NULL) {
        char *parts[500];
        int count = 0;
        for (char *part = strtok(line, " \t\r\n"); part != NULL;
             part = strtok(NULL, " \t\r\n")) {
            parts[count++] = part;
        }
        if (count == 0) {
            continue;
        }
        const char *command = parts[0];
        int got = count - 1;
        if (strcmp(command, "add") == 0) {
            if (takes(command, 2, got)) {
                add(&list, parts[1], parts[2]);
            }
        } else if (strcmp(command, "remove") == 0) {
            if (takes(command, 1, got)) {
                remove_song(&list, parts[1]);
            }
        } else if (strcmp(command, "rename") == 0) {
            if (takes(command, 2, got)) {
                rename_song(&list, parts[1], parts[2]);
            }
        } else if (strcmp(command, "show") == 0) {
            if (takes(command, 0, got)) {
                show(&list);
            }
        } else if (strcmp(command, "clear") == 0) {
            if (takes(command, 0, got)) {
                int cleared = clear(&list);
                printf("cleared %d song%s\n", cleared, cleared == 1 ? "" : "s");
            }
        } else {
            printf("error: unknown command \"%s\"\n", command);
        }
    }
    clear(&list);
    printf("blocks still allocated: %zu\n", live_blocks);
    return 0;
}
