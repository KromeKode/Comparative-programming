#include <stdbool.h>
#include <stdio.h>

#define MAX_EVENTS 8

// Every thing starts with this struct. The vtable lists the functions a kind
// of thing may replace; NULL means "use the default".
struct thing;
struct thing_vtable {
    void (*describe)(const struct thing *self);
};

struct thing {
    const struct thing_vtable *vtable;
    const char *name;
    char history[MAX_EVENTS][48];
    int events;
};

void describe_default(const struct thing *self) {
    printf("this is the %s", self->name);
}

void describe(const struct thing *self) {
    if (self->vtable->describe != NULL) {
        self->vtable->describe(self);
    } else {
        describe_default(self);
    }
    printf("\n");
}

void log_event(struct thing *self, const char *event) {
    if (self->events < MAX_EVENTS) {
        snprintf(self->history[self->events++], sizeof self->history[0],
                 "%s: %s", self->name, event);
    }
}

// A door uses every default.
static const struct thing_vtable door_vtable = {NULL};

void door_open(struct thing *door) { log_event(door, "opened"); }
void door_close(struct thing *door) { log_event(door, "closed"); }

// A light has its own data, and its own describe.
struct light {
    struct thing base;
    bool on;
};

void light_describe(const struct thing *self) {
    const struct light *light = (const struct light *)self;
    describe_default(self);
    printf(", and it is %s", light->on ? "on" : "off");
}

static const struct thing_vtable light_vtable = {light_describe};

void light_switch_on(struct light *light) {
    light->on = true;
    log_event(&light->base, "switched on");
}

int main(void) {
    struct thing door = {.vtable = &door_vtable, .name = "front door"};
    struct light light = {
        .base = {.vtable = &light_vtable, .name = "porch light"}};
    door_open(&door);
    door_close(&door);
    light_switch_on(&light);

    const struct thing *things[] = {&door, &light.base};
    for (int i = 0; i < 2; i++) {
        describe(things[i]);
        for (int j = 0; j < things[i]->events; j++) {
            printf("  %s\n", things[i]->history[j]);
        }
    }
    return 0;
}
